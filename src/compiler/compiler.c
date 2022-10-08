#include <compiler/compiler.h>
#include <common/common.h>
#include <parser/parser.h>
#include <object/object.h>

#ifdef DEBUG_PRINT_CODE
#include <debug/debug.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    Token name;
    int depth;
} Local;

typedef struct
{
    Local locals[UINT8_COUNT];
    int local_count;
    int scope_depth;
} Environment;

typedef struct
{
    bool failed;
} Compiler;

Compiler compiler;
Environment *current = NULL;
Chunk *compiling_chunk;

static void compiler_compile_expression(const SExpr *sexpr);
static void compiler_define_variable(uint8_t global);
static void compiler_compile_define(const SExpr *sexpr);

static Chunk *compiler_current_chunk()
{
    return compiling_chunk;
}

/* Error management */

static void compiler_failed_at(Token *token, const char *message)
{
    fprintf(stderr, "[%d:%d] Compiler failed", token->line, token->row);

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

static uint8_t compiler_identifier_constant(Token *name)
{
    return compiler_make_constant(VALUE_OBJ_VAL(object_copy_string(name->start,
                                                                   name->length)));
}

static bool compiler_identifiers_equal(Token *a, Token *b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int compiler_resolve_local(Environment *env, Token *name)
{
    for (int i = env->local_count - 1; i >= 0; i--)
    {
        Local *local = &env->locals[i];
        if (compiler_identifiers_equal(name, &local->name))
        {
            if (local->depth == -1)
            {
                compiler_failed_at(name, "Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static void compiler_add_local(Token name)
{
    if (current->local_count == UINT8_COUNT)
    {
        compiler_failed_at(&name, "Too many local variables in function.");
        return;
    }

    Local *local = &current->locals[current->local_count++];
    local->name = name;
    local->depth = -1;
}

static void compiler_declare_variable(Token name)
{
    for (int i = current->local_count - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth)
        {
            break;
        }

        if (compiler_identifiers_equal(&name, &local->name))
        {
            compiler_failed_at(&name, "Duplicate identifier in 'let' expression.");
        }
    }

    if (current->scope_depth == 0)
        return;

    compiler_add_local(name);
}

static void compiler_begin_scope()
{
    current->scope_depth++;
}

static void compiler_end_scope()
{
    current->scope_depth--;

    while (current->local_count > 0 &&
           current->locals[current->local_count - 1].depth > current->scope_depth)
    {
        compiler_emit_byte(OP_POP);
        current->local_count--;
    }
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

static uint8_t compiler_compile_variable(Token name)
{
    compiler_declare_variable(name);

    if (current->scope_depth > 0)
        return 0;

    return compiler_identifier_constant(&name);
}

static void compiler_mark_initialized()
{
    current->locals[current->local_count - 1].depth =
        current->scope_depth;
}

static void compiler_compile_named_variable(Token name, bool assign)
{
    uint8_t get_op, set_op;
    int arg = compiler_resolve_local(current, &name);

    if (arg != -1)
    {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    }
    else
    {
        arg = compiler_identifier_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (assign)
    {
        compiler_emit_bytes(set_op, (uint8_t)arg);
    }
    else
    {
        compiler_emit_bytes(get_op, (uint8_t)arg);
    }
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
        break;
    case TOKEN_SYMBOL:
        compiler_compile_named_variable(PARSER_AS_ATOM(sexpr), false);
        break;
    case TOKEN_STRING:
        compiler_compile_string(true, PARSER_AS_ATOM(sexpr));
        break;
    case TOKEN_TRUE:
        compiler_compile_boolean(sexpr, true);
        break;
    case TOKEN_FALSE:
        compiler_compile_boolean(sexpr, false);
        break;
    default:
        compiler_failed_at(&PARSER_AS_ATOM(PARSER_CAR(sexpr)),
                           "Unknown symbol.");
        break;
    }
}

static void compiler_compile_set_expression(const SExpr *sexpr)
{
    compiler_compile_expression(PARSER_CDDAR(sexpr));
    compiler_compile_named_variable(PARSER_AS_ATOM(PARSER_CDAR(sexpr)), true);
}

static void compiler_compile_let_expression(const SExpr *sexpr)
{
    SExpr *def;

    compiler_begin_scope();

    for (SExpr *bind = PARSER_CDAR(sexpr); !PARSER_IS_NULL(bind); bind = PARSER_CDR(bind))
    {
        uint8_t var = compiler_compile_variable(PARSER_AS_ATOM(PARSER_CAAR(bind)));
        compiler_compile_expression(PARSER_CADAR(bind));
        compiler_define_variable(var);
    }

    for (def = PARSER_CDDR(sexpr); compiler_is_definition(PARSER_CAR(def)); def = PARSER_CDR(def))
    {
        compiler_compile_define(PARSER_CAR(def));
    }

    for (SExpr *expr = def; !PARSER_IS_NULL(expr); expr = PARSER_CDR(expr))
    {
        compiler_compile_expression(PARSER_CAR(expr));
        if (!PARSER_IS_NULL(PARSER_CDR(expr)))
            compiler_emit_byte(OP_POP);
    }

    compiler_end_scope();
}

static void compiler_compile_begin_expression(const SExpr *sexpr)
{
    for (SExpr *expr = PARSER_CDR(sexpr); !PARSER_IS_NULL(expr); expr = PARSER_CDR(expr))
    {
        compiler_compile_expression(PARSER_CAR(expr));
        if (!PARSER_IS_NULL(PARSER_CDR(expr)))
            compiler_emit_byte(OP_POP);
    }
}

static void compiler_compile_compound_expression(const SExpr *sexpr)
{
    switch (PARSER_AS_ATOM(PARSER_CAR(sexpr)).type)
    {
    case TOKEN_SET:
        compiler_compile_set_expression(sexpr);
        break;
    case TOKEN_LET:
        compiler_compile_let_expression(sexpr);
        break;
    case TOKEN_BEGIN:
        compiler_compile_begin_expression(sexpr);
        break;
    default:
        compiler_failed_at(&PARSER_AS_ATOM(PARSER_CAR(sexpr)),
                           "Unknown symbol.");
        break;
    }
}

static void compiler_compile_expression(const SExpr *sexpr)
{
    if (PARSER_IS_CONS(sexpr))
    {
        compiler_compile_compound_expression(sexpr);
    }
    else
    {
        compiler_compile_atomic_expression(sexpr);
    }
}

static void compiler_define_variable(uint8_t global)
{
    if (current->scope_depth > 0)
    {
        compiler_mark_initialized();
        return;
    }
    compiler_emit_bytes(OP_DEFINE_GLOBAL, global);
}

static void compiler_compile_define(const SExpr *sexpr)
{
    uint8_t var = compiler_compile_variable(PARSER_AS_ATOM(PARSER_CDAR(sexpr)));
    compiler_compile_expression(PARSER_CDDAR(sexpr));
    compiler_define_variable(var);
}

static void compiler_compile_definition(const SExpr *sexpr)
{
    switch (PARSER_AS_ATOM(PARSER_CAR(sexpr)).type)
    {
    case TOKEN_DEFINE:
        compiler_compile_define(sexpr);
        break;
    }
}

static void compiler_compile_form(const SExpr *sexpr)
{
    SExprType sexpr_type = PARSER_TYPE(sexpr);

    if (compiler_is_definition(sexpr))
    {
        compiler_compile_definition(sexpr);
    }
    else
    {
        compiler_compile_expression(sexpr);
        // compiler_emit_byte(OP_POP);
    }
}

static void compiler_init_environment(Environment *env)
{
    env->local_count = 0;
    env->scope_depth = 0;
    current = env;
}

static void compiler_init_compiler()
{
    compiler.failed = false;
}

CompileResult compiler_compile(const char *source, Chunk *chunk)
{
    SExpr *sexpr;
    CompileResult result;
    compiling_chunk = chunk;
    Environment env;
    compiler_init_environment(&env);
    compiler_init_compiler();

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

        if (compiler.failed)
            result = COMPILER_COMPILE_ERROR;
    }

    return result;
}