#include <vm/vm.h>
#include <common/common.h>
#include <debug/debug.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <memory/memory.h>

#include <stdarg.h>
#include <stdio.h>

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

static InterpretResult vm_run()
{
#define VM_READ_BYTE() (*vm.ip++)
#define VM_READ_CONSTANT() (vm.chunk->constants.values[VM_READ_BYTE()])
#define VM_BINARY_OP(value_type, op)                                      \
    do                                                                    \
    {                                                                     \
        if (!VALUE_IS_NUMBER(vm_peek(0)) || !VALUE_IS_NUMBER(vm_peek(1))) \
        {                                                                 \
            vm_runtime_error("Operands must be numbers.");                \
            return INTERPRET_RUNTIME_ERROR;                               \
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
            value_print_value(vm_pop());
            printf("\n");
            return INTERPRET_OK;
        }
        }
    }

#undef VM_READ_BYTE
#undef VM_READ_CONSTANT
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

        if (compile_result == COMPILE_COMPILE_ERROR)
        {
            chunk_free_chunk(&chunk);
            return INTERPRET_COMPILE_ERROR;
        }
        if (compile_result == COMPILE_EOF)
        {
            chunk_free_chunk(&chunk);
            return INTERPRET_OK;
        }
        /*
        vm.chunk = &chunk;
        vm.ip = vm.chunk->code;
        interpret_result = vm_run();
        chunk_free_chunk(&chunk);
        */
        interpret_result = INTERPRET_OK;
        if (interpret_result != INTERPRET_OK)
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