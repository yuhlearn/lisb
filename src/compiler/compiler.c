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

static void compiler_parse_expression();

static Chunk *compiler_current_chunk()
{
    return compiling_chunk;
}

/* Error management */

static void parser_failed_at(Token *token, const char *message)
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

static void parser_failed(const char *message)
{
    parser_failed_at(&compiler.previous, message);
}

static void parser_failed_at_next(const char *message)
{
    parser_failed_at(&compiler.current, message);
}

/* Utility functions */

static void compiler_advance()
{
    compiler.previous = compiler.current;

    for (;;)
    {
        compiler.current = scanner_scan_token();
        if (compiler.current.type != TOKEN_FAIL)
            break;

        parser_failed_at_next(compiler.current.start);
    }
}

static void compiler_consume(TokenType type, const char *message)
{
    if (compiler.current.type == type)
    {
        compiler_advance();
        return;
    }

    parser_failed_at_next(message);
}

static bool parser_check(TokenType type)
{
    return compiler.current.type == type;
}

static bool compiler_match(TokenType type)
{
    if (!parser_check(type))
        return false;
    compiler_advance();
    return true;
}

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
        parser_failed("Too many constants in one chunk.");
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

/* Parsing */

static void compiler_parse_number()
{
    double value = strtod(compiler.previous.start, NULL);
    compiler_emit_constant(VALUE_NUMBER_VAL(value));
}

static void compiler_parse_string()
{
    compiler_emit_constant(VALUE_OBJ_VAL(object_copy_string(compiler.previous.start + 1,
                                                            compiler.previous.length - 2)));
}

static void compiler_parse_literal()
{
    switch (compiler.previous.type)
    {
    case TOKEN_FALSE:
        compiler_emit_byte(OP_FALSE);
        break;
    case TOKEN_NULL:
        compiler_emit_byte(OP_NULL);
        break;
    case TOKEN_TRUE:
        compiler_emit_byte(OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

static void compiler_parse_application()
{
    compiler_advance();
    TokenType token_type = compiler.previous.type;

    switch (token_type)
    {
    // Core expressions.
    case TOKEN_QUOTE:
    case TOKEN_LAMBDA:
    case TOKEN_IF:
    case TOKEN_SET:
    case TOKEN_CALL_CC:
        parser_failed("Core application not implemented.");
        return;

    // Function application.
    case TOKEN_SYMBOL:
        parser_failed("Procedure application not implemented.");
        return;

    default:
        parser_failed("Non-procedure in operator position.");
        return;
    }

    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void compiler_parse_expression()
{
    TokenType token_type = compiler.previous.type;

    switch (token_type)
    {
    case TOKEN_NUMBER:
        compiler_parse_number();
        return;
    case TOKEN_STRING:
        compiler_parse_string();
        return;
    case TOKEN_TRUE:
        compiler_parse_literal();
        return;
    case TOKEN_FALSE:
        compiler_parse_literal();
        return;
    case TOKEN_NULL:
        compiler_parse_literal();
        return;
    case TOKEN_SYMBOL:
        parser_failed("Variables not implemented.");
        return;
    case TOKEN_LEFT_PAREN:
        compiler_parse_application();
        return;
    default:
        parser_failed("Malformed expression.");
        return;
    }
}

static void compiler_parse_definition()
{
    TokenType token_type = compiler.previous.type;

    switch (token_type)
    {
    case TOKEN_DEFINE:

        return;
    }
}

static void compiler_parse_form()
{
    compiler_advance();
    compiler_parse_definition();
    compiler_parse_expression();
}

CompileResult compiler_compile(const char *source, Chunk *chunk)
{
    SExpr *sexpr;
    CompileResult result;

    result = parser_parse(&sexpr);

    if (result == COMPILE_OK)
    {
        // Compile
        // result = COMPILE_COMPILE_ERROR iff compilation fails

        parser_free_sexpr(sexpr);
    }

    return result;

    /*
    compiling_chunk = chunk;
    compiler.failed = false;
    compiler.panic_mode = false;

    while (!compiler_match(TOKEN_EOF))
    {
        compiler_parse_form();
    }

    compiler_end_compiler();
    return !compiler.failed;
    */
}