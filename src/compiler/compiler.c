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

/* Utility functions */

/* Emit code */

static void compiler_emit_byte(uint8_t byte)
{
    chunk_write_chunk(compiler_current_chunk(), byte, 0);
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
        // compiler_failed("Too many constants in one chunk.");
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

static bool compiler_is_definition(const SExpr *sexpr)
{
    if (PARSER_TYPE(sexpr) == SEXPR_CONS &&
        PARSER_CAR(sexpr)->type == SEXPR_ATOM)
    {
        switch (PARSER_AS_ATOM(PARSER_CAR(sexpr)).type)
        {
        case TOKEN_DEFINE:
            return true;
            break;
        }
    }

    return false;
}

static void compiler_compile_number(const SExpr *sexpr)
{
    double value = strtod(sexpr->value.atom.start, NULL);
    compiler_emit_constant(VALUE_NUMBER_VAL(value));
}

static void compiler_compile_symbol(bool can_assign, Token token)
{
    compiler_emit_constant(VALUE_OBJ_VAL(object_copy_string(token.start + 1,
                                                            token.length - 2)));
}

static void compiler_compile_string(bool can_assign, Token token)
{
    compiler_emit_constant(VALUE_OBJ_VAL(object_copy_string(token.start + 1,
                                                            token.length - 2)));
}

static void compiler_compile_boolean(const SExpr *sexpr, const bool value)
{
    compiler_emit_constant(VALUE_BOOL_VAL(value));
}

static void compiler_compile_atomic_expression(const SExpr *sexpr)
{
    switch (PARSER_AS_ATOM(sexpr).type)
    {
    case TOKEN_NUMBER:
        compiler_compile_number(sexpr);
        return;
    case TOKEN_SYMBOL:
    case TOKEN_STRING:
        compiler_compile_string(sexpr, PARSER_AS_ATOM(sexpr));
        return;
    case TOKEN_TRUE:
        compiler_compile_boolean(sexpr, true);
        return;
    case TOKEN_FALSE:
        compiler_compile_boolean(sexpr, false);
        return;
    }
}

static void compiler_compile_expression(const SExpr *sexpr)
{
    if (PARSER_IS_CONS(sexpr) == SEXPR_ATOM)
    {
        // compile parameterized expression
    }
    else
    {
        // compile atomic expression
        compiler_compile_atomic_expression(sexpr);
    }
}

static void compiler_compile_form(const SExpr *sexpr)
{
    SExprType sexpr_type = PARSER_TYPE(sexpr);

    if (compiler_is_definition(sexpr))
    {
        // compile definition
    }
    else
    {
        compiler_compile_expression(sexpr);
    }
}

static void compiler_init_compiler(Compiler *compiler)
{
    compiler->failed = false;
    compiler->panic_mode = false;
}

CompileResult compiler_compile(const char *source, Chunk *chunk)
{
    SExpr *sexpr;
    CompileResult result;
    compiling_chunk = chunk;

    result = parser_parse(&sexpr);

    if (result == COMPILER_OK)
    {
#ifdef DEBUG_PRINT_CODE
        printf("s-expr: ", source);
        debug_disassemble_sexpression(sexpr);
        printf("\n");
#endif
        compiler_compile_form(sexpr);
        compiler_end_compiler();
        // Compile
        // result = COMPILE_COMPILE_ERROR iff compilation fails
    }

    return result;
}