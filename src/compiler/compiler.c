#include <compiler/compiler.h>
#include <common/common.h>
#include <parser/parser.h>
#include <object/object.h>
#include <memory/memory.h>

#ifdef DEBUG_PRINT_CODE
#include <debug/debug.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
    ObjSymbol name;
    int depth;
    bool is_captured;
} Local;

typedef struct
{
    uint8_t index;
    bool is_local;
} Upvalue;

typedef enum
{
    TYPE_FUNCTION,
    TYPE_SCRIPT
} FunctionType;

typedef struct Environment
{
    struct Environment *enclosing;
    ObjFunction *function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int local_count;
    Upvalue upvalues[UINT8_COUNT];
    int scope_depth;
} Environment;

typedef struct
{
    bool failed;
} Compiler;

Compiler compiler;
Environment *current = NULL;
Chunk *compiling_chunk;

static void compiler_compile_expression(const Value sexpr, bool tail);
static void compiler_define_variable(const int global);
static void compiler_compile_define(const Value sexpr);

static Chunk *compiler_current_chunk()
{
    return &current->function->chunk;
}

/* Error management */

static void compiler_failed_at(ObjSymbol *symbol, const char *message)
{
    fprintf(stderr, "[%d:%d] Compiler failed at '%.*s': %s\n",
            symbol->line,
            symbol->row,
            symbol->length,
            symbol->chars,
            message);

    compiler.failed = true;
}

static void compiler_failed(const char *message)
{
    fprintf(stderr, "Compiler failed: %s\n", message);
    compiler.failed = true;
}

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

static void compiler_emit_short(uint16_t bytes)
{
    compiler_emit_byte((bytes >> 8) & 0xff);
    compiler_emit_byte(bytes & 0xff);
}

static int compiler_emit_jump(uint8_t instruction)
{
    compiler_emit_byte(instruction);
    compiler_emit_byte(0xff);
    compiler_emit_byte(0xff);
    return compiler_current_chunk()->count - 2;
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

static void compiler_patch_jump(int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = compiler_current_chunk()->count - offset - 2;

    if (jump > UINT16_MAX)
    {
        fprintf(stderr, "Too much code to jump over.");
        exit(1);
    }

    // write the jump offset to code using some bit manipulation
    compiler_current_chunk()->code[offset] = (jump >> 8) & 0xff;
    compiler_current_chunk()->code[offset + 1] = jump & 0xff;
}

static bool compiler_identifiers_equal(ObjSymbol *a, ObjSymbol *b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->chars, b->chars, a->length) == 0;
}

static void compiler_add_local(const Value symbol)
{
    ObjSymbol *name = OBJECT_AS_SYMBOL(symbol);

    if (current->local_count == UINT8_COUNT)
    {
        compiler_failed_at(OBJECT_AS_SYMBOL(symbol),
                           "Too many local variables in function.");
        return;
    }

    Local *local = &current->locals[current->local_count++];
    local->name = *name;
    local->depth = -1;
    local->is_captured = false;
}

static int compiler_resolve_local(Environment *env, ObjSymbol *name)
{
    for (int i = env->local_count - 1; i >= 0; i--)
    {
        Local *local = &env->locals[i];
        if (compiler_identifiers_equal(name, &local->name))
        {
            if (local->depth == -1)
            {
                compiler_failed("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static int compiler_add_upvalue(Environment *env, uint8_t index, bool is_local)
{
    int upvalue_count = env->function->upvalue_count;

    for (int i = 0; i < upvalue_count; i++)
    {
        Upvalue *upvalue = &env->upvalues[i];
        if (upvalue->index == index && upvalue->is_local == is_local)
        {
            return i;
        }
    }

    if (upvalue_count == UINT8_COUNT)
    {
        compiler_failed("Too many closure variables in function.");
        return 0;
    }

    env->upvalues[upvalue_count].is_local = is_local;
    env->upvalues[upvalue_count].index = index;
    return env->function->upvalue_count++;
}

static int compiler_resolve_upvalue(Environment *env, ObjSymbol *name)
{
    if (env->enclosing == NULL)
        return -1;

    int local = compiler_resolve_local(env->enclosing, name);

    if (local != -1)
    {
        env->enclosing->locals[local].is_captured = true;
        return compiler_add_upvalue(env, (uint8_t)local, true);
    }

    int upvalue = compiler_resolve_upvalue(env->enclosing, name);

    if (upvalue != -1)
    {
        return compiler_add_upvalue(env, (uint8_t)upvalue, false);
    }

    return -1;
}

static int compiler_declare_variable(const Value symbol)
{
    ObjSymbol *name = OBJECT_AS_SYMBOL(symbol);

    for (int i = current->local_count - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth)
        {
            break;
        }

        if (compiler_identifiers_equal(name, &local->name))
        {
            compiler_failed_at(OBJECT_AS_SYMBOL(symbol),
                               "Duplicate identifier in 'let' expression.");
        }
    }

    if (current->scope_depth == 0)
    {
        return table_declare(&vm.globals, object_copy_string(name->chars, name->length));
    }

    compiler_add_local(symbol);

    return -1;
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
        if (current->locals[current->local_count - 1].is_captured)
        {
            compiler_emit_byte(OP_CLOSE_UPVALUE);
        }
        else
        {
            compiler_emit_byte(OP_POP);
        }
        current->local_count--;
    }
}

/* Compilation */

static void compiler_init_environment(Environment *env, FunctionType type)
{
    env->enclosing = current;
    env->function = NULL;
    env->type = type;
    env->local_count = 0;
    env->scope_depth = 0;
    env->function = (type == TYPE_SCRIPT)
                        ? object_new_script()
                        : object_new_function();
    current = env;

    Local *local = &current->locals[current->local_count++];
    local->depth = 0;
    local->is_captured = false;
    local->name.chars = "";
    local->name.length = 0;
}

static ObjFunction *compiler_end_environment()
{
    compiler_emit_return();
    ObjFunction *function = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!compiler.failed)
    {
        char name[32] = {0};
        sprintf(name, "#<procedure %u>", (unsigned)current->function->id);
        debug_disassemble_chunk(compiler_current_chunk(), name);
    }
#endif

    current = current->enclosing;
    return function;
}

static bool compiler_is_definition(const Value sexpr)
{
    if (OBJECT_IS_CONS(sexpr) && OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)))
    {
        switch (OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token)
        {
        case TOKEN_DEFINE:
            return true;
        }
    }

    return false;
}

static void compiler_compile_number(const Value number)
{
    compiler_emit_constant(number);
}

static void compiler_mark_initialized()
{
    if (current->scope_depth == 0)
        return;
    current->locals[current->local_count - 1].depth =
        current->scope_depth;
}

static int compiler_resolve_global(Environment *current, const ObjSymbol *symbol)
{
    return table_find_entry(&vm.globals, symbol->chars, symbol->length);
}

static void compiler_compile_named_variable(const Value symbol, bool assign)
{
    uint8_t get_op, set_op;
    int arg = compiler_resolve_local(current, OBJECT_AS_SYMBOL(symbol));

    if (arg != -1)
    {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    }
    else if ((arg = compiler_resolve_upvalue(current, OBJECT_AS_SYMBOL(symbol))) != -1)
    {
        get_op = OP_GET_UPVALUE;
        set_op = OP_SET_UPVALUE;
    }
    else if ((arg = compiler_resolve_global(current, OBJECT_AS_SYMBOL(symbol))) != -1)
    {
        if (assign)
        {
            compiler_emit_byte(OP_SET_GLOBAL);
            compiler_emit_short((uint16_t)arg);
        }
        else
        {
            compiler_emit_byte(OP_GET_GLOBAL);
            compiler_emit_short((uint16_t)arg);
        }
        return;
    }
    else
    {
        compiler_failed_at(OBJECT_AS_SYMBOL(symbol), "Undefined variable.");
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

static void compiler_compile_string(const Value sexpr)
{
    ObjString *string = OBJECT_AS_STRING(sexpr);
    compiler_emit_constant(VALUE_OBJ_VAL(object_copy_string(string->chars,
                                                            string->length)));
}

static void compiler_compile_boolean(const Value sexpr, const bool value)
{
    compiler_emit_constant(VALUE_BOOL_VAL(value));
}

static void compiler_compile_atomic_expression(const Value sexpr)
{

    switch (sexpr.type)
    {
    case VALUE_NUMBER:
        compiler_compile_number(sexpr);
        break;
    case VALUE_BOOL:
        compiler_compile_boolean(sexpr, true);
        break;
    case VALUE_OBJ:
        switch (VALUE_AS_OBJ(sexpr)->type)
        {
        case OBJ_SYMBOL:
            compiler_compile_named_variable(sexpr, false);
            break;
        case OBJ_STRING:
            compiler_compile_string(sexpr);
            break;
        }
        break;
    default:
        compiler_failed("Unknown expression.");
        break;
    }
}

static void compiler_compile_lambda_expression(const Value sexpr)
{
    Value def;
    Environment env;

    compiler_init_environment(&env, TYPE_FUNCTION);

    compiler_begin_scope();

    for (Value formal = OBJECT_CDAR(sexpr); !VALUE_IS_NULL(formal); formal = OBJECT_CDR(formal))
    {
        current->function->arity++;
        if (current->function->arity > 255)
        {
            compiler_failed("Can't have more than 255 parameters.");
        }
        int var = compiler_declare_variable(OBJECT_CAR(formal));
        compiler_define_variable(var);
    }

    for (def = OBJECT_CDDR(sexpr); compiler_is_definition(OBJECT_CAR(def)); def = OBJECT_CDR(def))
    {
        compiler_compile_define(OBJECT_CAR(def));
    }

    for (Value expr = def; !VALUE_IS_NULL(expr); expr = OBJECT_CDR(expr))
    {
        if (!VALUE_IS_NULL(OBJECT_CDR(expr)))
        {
            compiler_compile_expression(OBJECT_CAR(expr), false);
            compiler_emit_byte(OP_POP);
        }
        else
        {
            compiler_compile_expression(OBJECT_CAR(expr), true);
        }
    }
    // End the environment and emit implicit return
    ObjFunction *function = compiler_end_environment();

    // Push the function onto the stack
    compiler_emit_bytes(OP_CLOSURE, compiler_make_constant(VALUE_OBJ_VAL(function)));

    for (int i = 0; i < function->upvalue_count; i++)
    {
        compiler_emit_byte(env.upvalues[i].is_local ? 1 : 0);
        compiler_emit_byte(env.upvalues[i].index);
    }
}

static void compiler_compile_set_expression(const Value sexpr)
{
    compiler_compile_expression(OBJECT_CDDAR(sexpr), false);
    compiler_compile_named_variable(OBJECT_CDAR(sexpr), true);
}

static void compiler_compile_let_expression(const Value sexpr, bool tail)
{
    Value def;

    compiler_begin_scope();

    // Save index for return value
    int result = current->local_count;

    for (Value bind = OBJECT_CDAR(sexpr); !VALUE_IS_NULL(bind); bind = OBJECT_CDR(bind))
    {
        int var = compiler_declare_variable(OBJECT_CAAR(bind));
        compiler_compile_expression(OBJECT_CADAR(bind), false);
        compiler_define_variable(var);
    }

    for (def = OBJECT_CDDR(sexpr); compiler_is_definition(OBJECT_CAR(def)); def = OBJECT_CDR(def))
    {
        compiler_compile_define(OBJECT_CAR(def));
    }

    for (Value expr = def; !VALUE_IS_NULL(expr); expr = OBJECT_CDR(expr))
    {
        compiler_compile_expression(OBJECT_CAR(expr), false);
        if (!VALUE_IS_NULL(OBJECT_CDR(expr)))
            compiler_emit_byte(OP_POP);
    }

    // Set the return value at return index
    compiler_emit_bytes(OP_SET_LOCAL, result);

    compiler_end_scope();
}

static void compiler_compile_begin_expression(const Value sexpr, bool tail)
{
    for (Value expr = OBJECT_CDR(sexpr); !VALUE_IS_NULL(expr); expr = OBJECT_CDR(expr))
    {
        if (!VALUE_IS_NULL(OBJECT_CDR(expr)))
        {
            compiler_compile_expression(OBJECT_CAR(expr), false);
            compiler_emit_byte(OP_POP);
        }
        else
        {
            compiler_compile_expression(OBJECT_CAR(expr), tail);
        }
    }
}

static void compiler_compile_if_expression(const Value sexpr)
{
    Value cond_expr, then_expr, else_expr;

    cond_expr = OBJECT_CDAR(sexpr);
    then_expr = OBJECT_CDDAR(sexpr);
    else_expr = OBJECT_CDDDAR(sexpr);

    compiler_compile_expression(cond_expr, false);

    int then_jump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    compiler_emit_byte(OP_POP);

    compiler_compile_expression(then_expr, false);

    int else_jump = compiler_emit_jump(OP_JUMP);
    compiler_patch_jump(then_jump);
    compiler_emit_byte(OP_POP);

    compiler_compile_expression(else_expr, false);

    compiler_patch_jump(else_jump);
}

static void compiler_compile_call_cc_expression(const Value sexpr)
{
    Value expr = OBJECT_CDAR(sexpr);
    uint8_t arg_count = 1;

    // Push the argument function first
    compiler_compile_expression(expr, false);

    // Create and push the current continuation
    compiler_emit_byte(OP_CONTINUATION);

    // Call argument function with continuation as argument
    compiler_emit_bytes(OP_CALL, arg_count);
}

static void compiler_compile_application_expression(const Value sexpr, bool tail)
{
    Value expr = sexpr;
    uint8_t arg_count = 0;

    compiler_compile_expression(OBJECT_CAR(expr), false);

    for (expr = OBJECT_CDR(expr); !VALUE_IS_NULL(expr); expr = OBJECT_CDR(expr))
    {
        compiler_compile_expression(OBJECT_CAR(expr), false);

        if (arg_count >= 255)
            compiler_failed("Can't have more than 255 arguments.");
        arg_count++;
    }

    compiler_emit_bytes((tail ? OP_TAIL_CALL : OP_CALL), arg_count);
}

static void compiler_compile_compound_expression(const Value sexpr, bool tail)
{
    if (OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)))
    {
        switch (OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token)
        {
        case TOKEN_LAMBDA:
            compiler_compile_lambda_expression(sexpr);
            return;
        case TOKEN_SET:
            compiler_compile_set_expression(sexpr);
            return;
        case TOKEN_LET:
            compiler_compile_let_expression(sexpr, tail);
            return;
        case TOKEN_BEGIN:
            compiler_compile_begin_expression(sexpr, tail);
            return;
        case TOKEN_IF:
            compiler_compile_if_expression(sexpr);
            return;
        case TOKEN_CALL_CC:
            compiler_compile_call_cc_expression(sexpr);
            return;
        }
    }
    compiler_compile_application_expression(sexpr, tail);
}

static void compiler_compile_expression(const Value sexpr, bool tail)
{
    if (OBJECT_IS_CONS(sexpr))
    {
        compiler_compile_compound_expression(sexpr, tail);
    }
    else
    {
        compiler_compile_atomic_expression(sexpr);
    }
}

static void compiler_define_variable(const int global)
{
    if (current->scope_depth > 0)
    {
        compiler_mark_initialized();
        return;
    }
    compiler_emit_byte(OP_SET_GLOBAL);
    compiler_emit_short((uint16_t)global);
}

static void compiler_compile_define(const Value sexpr)
{
    int var = compiler_declare_variable(OBJECT_CDAR(sexpr));
    compiler_compile_expression(OBJECT_CDDAR(sexpr), false);
    compiler_define_variable(var);
}

static void compiler_compile_definition(const Value sexpr)
{
    switch (OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token)
    {
    case TOKEN_DEFINE:
        compiler_compile_define(sexpr);
        break;
    }
}

static void compiler_compile_form(const Value sexpr)
{
    if (compiler_is_definition(sexpr))
    {
        compiler_compile_definition(sexpr);
    }
    else
    {
        compiler_compile_expression(sexpr, false);
    }
}

ObjFunction *compiler_compile(const char *source)
{
    Environment env;

    compiler.failed = false;

    Value sexpr = parser_parse();

    if (!VALUE_IS_VOID(sexpr))
    {
        compiler_init_environment(&env, TYPE_SCRIPT);

#ifdef DEBUG_PRINT_CODE
        printf("\ns-expr: ");
        value_print_value(sexpr);
        printf("\n\n");
#endif

        compiler_compile_form(sexpr);
        ObjFunction *function = compiler_end_environment();

        if (!compiler.failed)
        {
            return function;
        }
    }
    return NULL;
}

void compiler_mark_compiler_roots()
{
    Environment *env = current;

    while (env != NULL)
    {
        memory_mark_object((Obj *)env->function);
        env = env->enclosing;
    }
}