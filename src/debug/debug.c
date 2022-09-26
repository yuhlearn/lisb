#include <debug/debug.h>
#include <value/value.h>

#include <stdio.h>

void debug_disassemble_chunk(Chunk *chunk, const char *name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;)
    {
        offset = debug_disassemble_instruction(chunk, offset);
    }
}

static int debug_constant_instruction(const char *name, Chunk *chunk,
                                      int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    value_print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

static int debug_simple_instruction(const char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

int debug_disassemble_instruction(Chunk *chunk, int offset)
{
    printf("%04d ", offset);

    if (offset > 0 &&
        chunk->lines[offset] == chunk->lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction)
    {
    case OP_CONSTANT:
        return debug_constant_instruction("OP_CONSTANT", chunk, offset);
    case OP_NULL:
        return debug_simple_instruction("OP_NULL", offset);
    case OP_TRUE:
        return debug_simple_instruction("OP_TRUE", offset);
    case OP_FALSE:
        return debug_simple_instruction("OP_FALSE", offset);
    case OP_ADD:
        return debug_simple_instruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return debug_simple_instruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return debug_simple_instruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return debug_simple_instruction("OP_DIVIDE", offset);
    case OP_RETURN:
        return debug_simple_instruction("OP_RETURN", offset);
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}