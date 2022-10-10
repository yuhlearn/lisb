#include <debug/debug.h>
#include <value/value.h>
#include <scanner/scanner.h>

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

static int debug_byte_instruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
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
        return debug_constant_instruction("OP_GET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL:
        return debug_constant_instruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
        return debug_constant_instruction("OP_SET_GLOBAL", chunk, offset);
    case OP_ADD:
        return debug_simple_instruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return debug_simple_instruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return debug_simple_instruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return debug_simple_instruction("OP_DIVIDE", offset);
    case OP_JUMP:
        return debug_jump_instruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return debug_jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_CALL:
        return debug_byte_instruction("OP_CALL", chunk, offset);
    case OP_RETURN:
        return debug_simple_instruction("OP_RETURN", offset);
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}

void debug_print_sexpression(SExpr *sexpr)
{
    if (sexpr == NULL)
    {
        printf("NULL ");
        return;
    }

    SExprType type = sexpr->type;

    switch (type)
    {
    case SEXPR_ATOM:
        printf("%.*s ", sexpr->value.atom.length, sexpr->value.atom.start);
        break;
    case SEXPR_CONS:
        debug_disassemble_sexpression(PARSER_CAR(sexpr));
        debug_print_sexpression(PARSER_CDR(sexpr));
        break;
    case SEXPR_NULL:
        printf(") ");
        break;
    }
    return;
}

void debug_disassemble_sexpression(SExpr *sexpr)
{
    if (sexpr == NULL)
    {
        printf("NULL ");
        return;
    }
    else if (sexpr->type == SEXPR_NULL)
    {
        printf("( ) ");
        return;
    }
    else if (sexpr->type == SEXPR_CONS)
    {
        printf("( ");
    }

    debug_print_sexpression(sexpr);
}