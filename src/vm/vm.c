#include <vm/vm.h>
#include <common/common.h>
#include <debug/debug.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <memory/memory.h>
#include <primitive/primitive.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

VM vm;

static void vm_reset_stack()
{
    vm.stack_top = vm.stack;
    vm.frame_count = 0;
    vm.open_upvalues = NULL;
}

void vm_free_vm()
{
    table_free_table(&vm.strings);
    memory_free_objects();
}

void vm_push(Value value)
{
    *vm.stack_top = value;
    vm.stack_top++;
}

Value vm_pop()
{
    vm.stack_top--;
    return *vm.stack_top;
}

static Value vm_peek(int distance)
{
    return vm.stack_top[-1 - distance];
}

void vm_runtime_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // Print the stack trace
    for (int i = vm.frame_count - 1; i >= 0; i--)
    {
        CallFrame *frame = &vm.call_frames[i];
        ObjFunction *function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;

        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        printf("#<procedure %u>\n", (unsigned)function->id);
    }

    vm_reset_stack();
}

static void vm_define_primitive(const char *name, PrimitiveFn function)
{
    vm_push(VALUE_OBJ_VAL(object_copy_string(name, (int)strlen(name))));
    vm_push(VALUE_OBJ_VAL(object_new_native(function)));
    int slot = table_declare(&vm.globals, OBJECT_AS_STRING(vm.stack[0]));
    table_set(&vm.globals, slot, vm.stack[1]);
    vm_pop();
    vm_pop();
}

void vm_init_vm()
{
    vm_reset_stack();
    memory_init_memory();

    table_init_table(&vm.globals);
    table_init_table(&vm.strings);

    vm_define_primitive("clock", primitive_clock);
    vm_define_primitive("display", primitive_display);
    vm_define_primitive("displayln", primitive_displayln);

    vm_define_primitive("+", primitive_add);
    vm_define_primitive("-", primitive_sub);
    vm_define_primitive("*", primitive_mup);
    vm_define_primitive("/", primitive_div);

    vm_define_primitive("=", primitive_num_eq);
    vm_define_primitive("<", primitive_num_le);
    vm_define_primitive(">", primitive_num_ge);
    vm_define_primitive("<=", primitive_num_leq);
    vm_define_primitive(">=", primitive_num_geq);

    vm_define_primitive("car", primitive_car);
    vm_define_primitive("cdr", primitive_cdr);
    vm_define_primitive("cons", primitive_cons);
    vm_define_primitive("list", primitive_list);
    vm_define_primitive("append", primitive_append);
}

static bool vm_call(ObjClosure *closure, int arg_count)
{
    if (arg_count != closure->function->arity)
    {
        vm_runtime_error("Expected %d arguments but got %d.",
                         closure->function->arity, arg_count);
        return false;
    }

    if (vm.frame_count == VM_FRAMES_MAX)
    {
        vm_runtime_error("Stack overflow.");
        return false;
    }

    CallFrame *frame = &vm.call_frames[vm.frame_count++];

    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stack_top - arg_count - 1;

    return true;
}

static bool vm_call_value(Value callee, int arg_count)
{
    if (VALUE_IS_OBJ(callee))
    {
        switch (OBJECT_OBJ_TYPE(callee))
        {
        case OBJ_CLOSURE:
        {
            return vm_call(OBJECT_AS_CLOSURE(callee), arg_count);
        }
        case OBJ_PRIMITIVE:
        {
            PrimitiveFn primitive = OBJECT_AS_PRIMITIVE(callee);
            Value result = primitive(arg_count, vm.stack_top - arg_count);
            vm.stack_top -= arg_count + 1;
            vm_push(result);
            return true;
        }
        case OBJ_CONTINUATION:
        {
            if (arg_count != 1)
            {
                vm_runtime_error("Expected %d arguments but got %d.",
                                 1, arg_count);
                return false;
            }

            ObjContinuation *cont = OBJECT_AS_CONTINUATION(callee);
            Value result = vm_pop();

            object_load_continuation(cont);

            vm.call_frames[vm.frame_count - 1].ip += 2; // Skip the call/cc call

            vm_pop();        // The inital procedure
            vm_push(result); // The new return value

            return true;
        }
        default:
            break; // Non-callable object type.
        }
    }
    vm_runtime_error("Application not a procedure.");
    return false;
}

static ObjUpvalue *vm_capture_upvalue(Value *local)
{
    ObjUpvalue *prev_upvalue = NULL;
    ObjUpvalue *upvalue = vm.open_upvalues;

    while (upvalue != NULL && upvalue->location > local)
    {
        prev_upvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local)
    {
        return upvalue;
    }

    ObjUpvalue *created_upvalue = object_new_upvalue(local);
    created_upvalue->next = upvalue;

    if (prev_upvalue == NULL)
    {
        vm.open_upvalues = created_upvalue;
    }
    else
    {
        prev_upvalue->next = created_upvalue;
    }

    return created_upvalue;
}

static void vm_close_upvalues(Value *last)
{
    while (vm.open_upvalues != NULL &&
           vm.open_upvalues->location >= last)
    {
        ObjUpvalue *upvalue = vm.open_upvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.open_upvalues = upvalue->next;
    }
}

static bool vm_is_falsey(Value value)
{
    return VALUE_IS_BOOL(value) && !VALUE_AS_BOOL(value);
}

static void vm_concatenate()
{
    ObjString *b = OBJECT_AS_STRING(vm_pop());
    ObjString *a = OBJECT_AS_STRING(vm_pop());

    int length = a->length + b->length;
    char *chars = MEMORY_ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = object_take_string(chars, length);
    vm_push(VALUE_OBJ_VAL(result));
}

static InterpretResult vm_run()
{
    CallFrame *frame = &vm.call_frames[vm.frame_count - 1];

#define VM_READ_BYTE() (*frame->ip++)
#define VM_READ_CONSTANT() (frame->closure->function->chunk.constants.values[VM_READ_BYTE()])
#define VM_READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define VM_READ_STRING() OBJECT_AS_STRING(VM_READ_CONSTANT())
    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = vm.stack; slot < vm.stack_top; slot++)
        {
            printf("[ ");
            value_print_value(*slot);
            printf(" ]");
        }
        printf("\n");

        debug_disassemble_instruction(&frame->closure->function->chunk,
                                      (int)(frame->ip - frame->closure->function->chunk.code));
#endif
        uint8_t instruction;

        switch (instruction = VM_READ_BYTE())
        {
        case OP_CONSTANT:
        {
            Value constant = VM_READ_CONSTANT();
            vm_push(constant);
            break;
        }
        case OP_NULL:
        {
            vm_push(VALUE_NULL_VAL);
            break;
        }
        case OP_TRUE:
        {
            vm_push(VALUE_BOOL_VAL(true));
            break;
        }
        case OP_FALSE:
        {
            vm_push(VALUE_BOOL_VAL(false));
            break;
        }
        case OP_POP:
        {
            vm_pop();
            break;
        }
        case OP_GET_LOCAL:
        {
            uint8_t slot = VM_READ_BYTE();
            vm_push(frame->slots[slot]);
            break;
        }
        case OP_GET_GLOBAL:
        {
            uint16_t slot = VM_READ_SHORT();
            vm_push(table_get(&vm.globals, slot));
            break;
        }
        case OP_GET_UPVALUE:
        {
            uint8_t slot = VM_READ_BYTE();
            vm_push(*frame->closure->upvalues[slot]->location);
            break;
        }
        case OP_SET_LOCAL:
        {
            uint8_t slot = VM_READ_BYTE();
            frame->slots[slot] = vm_peek(0);
            break;
        }
        case OP_SET_GLOBAL:
        {
            uint16_t slot = VM_READ_SHORT();
            table_set(&vm.globals, slot, vm_peek(0));
            break;
        }
        case OP_SET_UPVALUE:
        {
            uint8_t slot = VM_READ_BYTE();
            *frame->closure->upvalues[slot]->location = vm_peek(0);
            break;
        }
        case OP_JUMP:
        {
            uint16_t offset = VM_READ_SHORT();
            frame->ip += offset;
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            uint16_t offset = VM_READ_SHORT();
            if (vm_is_falsey(vm_peek(0)))
                frame->ip += offset;
            break;
        }
        case OP_CALL:
        {
            int arg_count = VM_READ_BYTE();

            if (!vm_call_value(vm_peek(arg_count), arg_count))
            {
                return VM_RUNTIME_ERROR;
            }
            frame = &vm.call_frames[vm.frame_count - 1];
            break;
        }
        case OP_TAIL_CALL:
        {
            int arg_count = VM_READ_BYTE();

            // Close upvalues
            vm_close_upvalues(frame->slots);

            // Shift the arguments plus closure to be called
            memcpy(frame->slots,
                   vm.stack_top - (arg_count + 1),
                   sizeof(Value) * (arg_count + 1));

            // Restore the stack top and pop the current call frame
            vm.stack_top = frame->slots + arg_count + 1;
            vm.frame_count--;
            frame = &vm.call_frames[vm.frame_count - 1];

            // Call as normal
            if (!vm_call_value(vm_peek(arg_count), arg_count))
            {
                return VM_RUNTIME_ERROR;
            }
            frame = &vm.call_frames[vm.frame_count - 1];
            break;
        }
        case OP_CLOSURE:
        {
            ObjFunction *function = OBJECT_AS_FUNCTION(VM_READ_CONSTANT());
            ObjClosure *closure = object_new_closure(function);
            vm_push(VALUE_OBJ_VAL(closure));

            for (int i = 0; i < closure->upvalue_count; i++)
            {
                uint8_t is_local = VM_READ_BYTE();
                uint8_t index = VM_READ_BYTE();

                if (is_local)
                {
                    closure->upvalues[i] = vm_capture_upvalue(frame->slots + index);
                }
                else
                {
                    closure->upvalues[i] = frame->closure->upvalues[index];
                }
            }
            break;
        }
        case OP_CONTINUATION:
        {
            ObjContinuation *cont = object_new_continuation((struct VM *)&vm);
            vm_push(VALUE_OBJ_VAL(cont));
            break;
        }
        case OP_CLOSE_UPVALUE:
        {
            vm_close_upvalues(vm.stack_top - 1);
            vm_pop();
            break;
        }
        case OP_RETURN:
        {
            Value result = vm_pop();
            vm_close_upvalues(frame->slots);
            vm.frame_count--;

            if (vm.frame_count == 0)
            {
                vm_pop();
                value_print_value(result);
                printf("\n");
                return VM_OK;
            }

            vm.stack_top = frame->slots;
            vm_push(result);
            frame = &vm.call_frames[vm.frame_count - 1];
            break;
        }
        }
    }

#undef VM_READ_BYTE
#undef VM_READ_CONSTANT
#undef VM_READ_SHORT
#undef VM_READ_STRING
}

InterpretResult vm_interpret(const char *source)
{
    parser_init_parser(source);

    do
    {
        ObjFunction *function = compiler_compile(source);

        if (function == NULL)
        {
            if (PARSER_IS_EOF(parser_get_error_token()))
                return VM_OK;
            return VM_COMPILE_ERROR;
        }

        vm_push(VALUE_OBJ_VAL(function));
        ObjClosure *closure = object_new_closure(function);
        vm_pop();
        vm_push(VALUE_OBJ_VAL(closure));
        vm_call(closure, 0);

        if (vm_run() == VM_RUNTIME_ERROR)
            return VM_RUNTIME_ERROR;
    } while (true);
}