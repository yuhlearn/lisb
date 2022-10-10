#include <vm/vm.h>
#include <common/common.h>
#include <debug/debug.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <memory/memory.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

VM vm;

static void vm_reset_stack()
{
    vm.stack_top = vm.stack;
    vm.frame_count = 0;
}

static void vm_runtime_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // Print the stack trace
    for (int i = vm.frame_count - 1; i >= 0; i--)
    {
        CallFrame *frame = &vm.frames[i];
        ObjFunction *function = frame->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);

        if (function->name == NULL)
        {
            fprintf(stderr, "script\n");
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    vm_reset_stack();
}

void vm_init_vm()
{
    vm_reset_stack();
    vm.objects = NULL;
    table_init_table(&vm.globals);
    table_init_table(&vm.strings);
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

static bool vm_call(ObjFunction *function, int arg_count)
{
    if (arg_count != function->arity)
    {
        vm_runtime_error("Expected %d arguments but got %d.",
                         function->arity, arg_count);
        return false;
    }

    if (vm.frame_count == VM_FRAMES_MAX)
    {
        vm_runtime_error("Stack overflow.");
        return false;
    }

    CallFrame *frame = &vm.frames[vm.frame_count++];

    frame->function = function;
    frame->ip = function->chunk.code;
    frame->slots = vm.stack_top - arg_count - 1;

    return true;
}

static bool vm_call_value(Value callee, int arg_count)
{
    if (VALUE_IS_OBJ(callee))
    {
        switch (OBJECT_OBJ_TYPE(callee))
        {
        case OBJ_FUNCTION:
            return vm_call(OBJECT_AS_FUNCTION(callee), arg_count);
        default:
            break; // Non-callable object type.
        }
    }
    vm_runtime_error("Application not a procedure.");
    return false;
}

static bool vm_is_falsey(Value value)
{
    return VALUE_IS_BOOL(value) && !VALUE_AS_BOOL(value);
}

static void concatenate()
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
    CallFrame *frame = &vm.frames[vm.frame_count - 1];

#define VM_READ_BYTE() (*frame->ip++)
#define VM_READ_CONSTANT() (frame->function->chunk.constants.values[VM_READ_BYTE()])
#define VM_READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define VM_READ_STRING() OBJECT_AS_STRING(VM_READ_CONSTANT())
#define VM_BINARY_OP(value_type, op)                                      \
    do                                                                    \
    {                                                                     \
        if (!VALUE_IS_NUMBER(vm_peek(0)) || !VALUE_IS_NUMBER(vm_peek(1))) \
        {                                                                 \
            vm_runtime_error("Operands must be numbers.");                \
            return VM_RUNTIME_ERROR;                                      \
        }                                                                 \
        double b = VALUE_AS_NUMBER(vm_pop());                             \
        double a = VALUE_AS_NUMBER(vm_pop());                             \
        vm_push(value_type(a op b));                                      \
    } while (false)

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

        debug_disassemble_instruction(&frame->function->chunk,
                                      (int)(frame->ip - frame->function->chunk.code));
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
            vm_push(VALUE_NULL_VAL);
            break;
        case OP_TRUE:
            vm_push(VALUE_BOOL_VAL(true));
            break;
        case OP_FALSE:
            vm_push(VALUE_BOOL_VAL(false));
            break;
        case OP_POP:
            vm_pop();
            break;
        case OP_GET_LOCAL:
        {
            uint8_t slot = VM_READ_BYTE();
            vm_push(frame->slots[slot]);
            break;
        }
        case OP_GET_GLOBAL:
        {
            ObjString *name = VM_READ_STRING();
            Value value;
            if (!table_get(&vm.globals, name, &value))
            {
                vm_runtime_error("Undefined variable '%s'.", name->chars);
                return VM_RUNTIME_ERROR;
            }
            vm_push(value);
            break;
        }
        case OP_DEFINE_GLOBAL:
        {
            ObjString *name = VM_READ_STRING();
            table_set(&vm.globals, name, vm_peek(0));
            vm_pop();
            vm_push(VALUE_VOID_VAL);
            break;
        }
        case OP_SET_LOCAL:
        {
            uint8_t slot = VM_READ_BYTE();
            frame->slots[slot] = vm_pop(0);
            vm_push(VALUE_VOID_VAL);
            break;
        }
        case OP_SET_GLOBAL:
        {
            ObjString *name = VM_READ_STRING();
            if (table_set(&vm.globals, name, vm_pop()))
            {
                table_delete(&vm.globals, name);
                vm_runtime_error("Undefined variable '%s'.", name->chars);
                return VM_RUNTIME_ERROR;
            }
            vm_push(VALUE_VOID_VAL);
            break;
        }
        case OP_ADD:
            VM_BINARY_OP(VALUE_NUMBER_VAL, +);
            break;
        case OP_SUBTRACT:
            VM_BINARY_OP(VALUE_NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            VM_BINARY_OP(VALUE_NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            VM_BINARY_OP(VALUE_NUMBER_VAL, /);
            break;
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
            frame = &vm.frames[vm.frame_count - 1];
            break;
        }
        case OP_RETURN:
        {
            Value result = vm_pop();
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
            frame = &vm.frames[vm.frame_count - 1];
            break;
        }
        }
    }

#undef VM_READ_BYTE
#undef VM_READ_CONSTANT
#undef VM_READ_SHORT
#undef VM_READ_STRING
#undef VM_BINARY_OP
}

InterpretResult vm_interpret(const char *source)
{
    ObjFunction *function = compiler_compile(source);
    if (function == NULL)
        return VM_COMPILE_ERROR;

    vm_push(VALUE_OBJ_VAL(function));
    vm_call(function, 0);

    return vm_run();
}