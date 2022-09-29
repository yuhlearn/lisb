#include <compiler/compiler.h>
#include <common/common.h>
#include <parser/parser.h>
#include <object/object.h>

#ifdef DEBUG_PRINT_CODE
#include <debug/debug.h>
#endif

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    Token current;
    Token previous;
    bool failed;
    bool panic_mode;
} Compiler;

Compiler compiler;
Chunk *compiling_chunk;

static Chunk *compiler_current_chunk()
{
    return compiling_chunk;
}

/* Error management */

static void compiler_failed_at(Token *token, const char *message)
{
    if (compiler.panic_mode)
        return;
    compiler.panic_mode = true;

    fprintf(stderr, "[line %d] Failed", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_FAIL)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    compiler.failed = true;
}

static void compiler_failed(const char *message)
{
    compiler_failed_at(&compiler.previous, message);
}

static void compiler_failed_at_next(const char *message)
{
    compiler_failed_at(&compiler.current, message);
}

/* Utility functions */

/* Emit code */

static void compiler_emit_byte(uint8_t byte)
{
    chunk_write_chunk(compiler_current_chunk(), byte, compiler.previous.line);
}

static void compiler_emit_bytes(uint8_t byte1, uint8_t byte2)
{
    compiler_emit_byte(byte1);
    compiler_emit_byte(byte2);
}

static void compiler_emit_return()
{
    compiler_emit_byte(OP_RETURN);
}

static uint8_t compiler_make_constant(Value value)
{
    int constant = chunk_add_constant(compiler_current_chunk(), value);
    if (constant > UINT8_MAX)
    {
        compiler_failed("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void compiler_emit_constant(Value value)
{
    compiler_emit_bytes(OP_CONSTANT, compiler_make_constant(value));
}

static void compiler_end_compiler()
{
    compiler_emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!compiler.failed)
    {
        debug_disassemble_chunk(compiler_current_chunk(), "code");
    }
#endif
}

/* Compilation */

CompileResult compiler_compile(const char *source, Chunk *chunk)
{
    SExpr *sexpr;
    CompileResult result;

    result = parser_parse(&sexpr);

#ifdef DEBUG_PRINT_CODE
    if (result == COMPILE_OK)
    {
        printf("s-expr: ", source);
        debug_disassemble_sexpression(sexpr);
        printf("\n");
    }
#endif

    // Compile
    // result = COMPILE_COMPILE_ERROR iff compilation fails

    return result;
}