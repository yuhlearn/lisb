#include <debug/debug.h>
#include <value/value.h>
#include <scanner/scanner.h>
#include <object/object.h>

#include <stdio.h>

void debug_disassemble_chunk(Chunk *chunk, const char *name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;)
    {
        offset = debug_disassemble_instruction(chunk, offset);
    }

    printf("\n");
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

static int debug_byte_instruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

static int debug_short_instruction(const char *name, Chunk *chunk, int offset)
{
    uint16_t slot = (uint16_t)(chunk->code[offset + 1] << 8);
    slot |= chunk->code[offset + 2];
    printf("%-16s %4u\n", name, (unsigned)slot);
    return offset + 3;
}

static int debug_jump_instruction(const char *name, int sign,
                                  Chunk *chunk, int offset)
{
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
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
    case OP_POP:
        return debug_simple_instruction("OP_POP", offset);
    case OP_GET_LOCAL:
        return debug_byte_instruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
        return debug_byte_instruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:
        return debug_short_instruction("OP_GET_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
        return debug_short_instruction("OP_SET_GLOBAL", chunk, offset);
    case OP_GET_UPVALUE:
        return debug_byte_instruction("OP_GET_UPVALUE", chunk, offset);
    case OP_SET_UPVALUE:
        return debug_byte_instruction("OP_SET_UPVALUE", chunk, offset);
    case OP_JUMP:
        return debug_jump_instruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return debug_jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_CALL:
        return debug_byte_instruction("OP_CALL", chunk, offset);
    case OP_TAIL_CALL:
        return debug_byte_instruction("OP_TAIL_CALL", chunk, offset);
    case OP_CLOSURE:
    {
        offset++;
        uint8_t constant = chunk->code[offset++];
        printf("%-16s %4d ", "OP_CLOSURE", constant);
        value_print_value(chunk->constants.values[constant]);
        printf("\n");

        ObjFunction *function = OBJECT_AS_FUNCTION(
            chunk->constants.values[constant]);
        for (int j = 0; j < function->upvalue_count; j++)
        {
            int is_local = chunk->code[offset++];
            int index = chunk->code[offset++];
            printf("%04d    |                     %s %d\n",
                   offset - 2, is_local ? "local" : "upvalue", index);
        }

        return offset;
    }
    case OP_CONTINUATION:
        return debug_simple_instruction("OP_CONTINUATION", offset);
    case OP_CLOSE_UPVALUE:
        return debug_simple_instruction("OP_CLOSE_UPVALUE", offset);
    case OP_RETURN:
        return debug_simple_instruction("OP_RETURN", offset);
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}