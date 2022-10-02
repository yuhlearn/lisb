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
}

static void vm_runtime_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
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
#define VM_READ_BYTE() (*vm.ip++)
#define VM_READ_CONSTANT() (vm.chunk->constants.values[VM_READ_BYTE()])
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

        debug_disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
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
        case OP_RETURN:
        {
            // value_print_value(vm_pop());
            printf("\n");
            return VM_OK;
        }
        }
    }

#undef VM_READ_BYTE
#undef VM_READ_CONSTANT
#undef VM_READ_STRING
#undef VM_BINARY_OP
}

InterpretResult vm_interpret(const char *source)
{
    CompileResult compile_result;
    InterpretResult interpret_result;
    Chunk chunk;

    parser_init_parser(source);
    for (;;)
    {
        chunk_init_chunk(&chunk);
        compile_result = compiler_compile(source, &chunk);

        if (compile_result != COMPILER_OK)
        {
            parser_free_sexpr();
            chunk_free_chunk(&chunk);

            if (compile_result == COMPILER_COMPILE_ERROR)
                return VM_COMPILE_ERROR;

            // EOF - exit compiler successfully
            return VM_OK;
        }

        vm.chunk = &chunk;
        vm.ip = vm.chunk->code;
        interpret_result = vm_run();
        chunk_free_chunk(&chunk);

        if (interpret_result == VM_RUNTIME_ERROR)
            return interpret_result;
    }
    parser_free_sexpr();
    /*
    if (!compiler_compile(source, &chunk))
    {
        chunk_free_chunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = vm_run();

    chunk_free_chunk(&chunk);
    return result;
    */
}