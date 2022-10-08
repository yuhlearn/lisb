#include <parser/parser.h>
#include <memory/memory.h>
#include <common/common.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int capacity;
    int count;
    SExpr *sexprs;
} SexprArray;

typedef struct
{
    Token lookahead;
    Token this;
    Token error_token;
    SexprArray sexpr_array;
} Parser;

Parser parser;

static SExpr *parser_parse_list();
static SExpr *parser_parse_formals();
static SExpr *parser_parse_body();
static SExpr *parser_parse_datum();
static SExpr *parser_parse_define();
static SExpr *parser_parse_definition();
static SExpr *parser_parse_quote();
static SExpr *parser_parse_lambda();
static SExpr *parser_parse_if();
static SExpr *parser_parse_set();
static SExpr *parser_parse_call_cc();
static SExpr *parser_parse_application();
static SExpr *parser_parse_expression();
static SExpr *parser_parse_form();

/* Error management */

Token parser_get_error_token()
{
    return parser.error_token;
}

static SExpr *parser_failed_at(Token *token, const char *message)
{
    // fprintf(stderr, "Parser error: ");
    fprintf(stderr, "[%d:%d] Parser failed", token->line, token->row);

    parser.error_token = *token;

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_FAIL)
    {
        // Nothing
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);

    return NULL;
}

static SExpr *parser_failed(const char *message)
{
    return parser_failed_at(&parser.this, message);
}

/* Parser -- utility functions */

static void parser_advance()
{
    parser.this = parser.lookahead;
    parser.lookahead = scanner_scan_token();
}

static bool parser_check(TokenType type)
{
    return parser.lookahead.type == type;
}

static bool parser_match(TokenType type)
{
    if (!parser_check(type))
        return false;
    parser_advance();
    return true;
}

static bool parser_is_definition()
{
    if (parser.this.type == TOKEN_LEFT_PAREN)
        switch (parser.lookahead.type)
        {
        case TOKEN_DEFINE:
            return true;
        }

    return false;
}

/* Manage the s-expression array */

static SExpr *parser_write_sexpr_array(SExpr value)
{
    if (parser.sexpr_array.capacity < parser.sexpr_array.count + 1)
    {
        int old_capacity = parser.sexpr_array.capacity;
        parser.sexpr_array.capacity = MEMORY_GROW_CAPACITY(old_capacity);
        parser.sexpr_array.sexprs = MEMORY_GROW_ARRAY(SExpr,
                                                      parser.sexpr_array.sexprs,
                                                      old_capacity,
                                                      parser.sexpr_array.capacity);
    }

    parser.sexpr_array.sexprs[parser.sexpr_array.count] = value;
    SExpr *sexpr = parser.sexpr_array.sexprs + parser.sexpr_array.count;
    parser.sexpr_array.count++;
    return sexpr;
}

static SExpr *parser_write_atom(Token token)
{
    SExpr atom, *sexpr;
    atom.type = SEXPR_ATOM;
    atom.value.atom = token;
    sexpr = parser_write_sexpr_array(atom);
    parser_advance();
    return sexpr;
}

static SExpr *parser_write_null()
{
    SExpr null;
    null.type = SEXPR_NULL;
    return parser_write_sexpr_array(null);
}

static SExpr *parser_write_cons()
{
    SExpr cons;
    cons.type = SEXPR_CONS;
    PARSER_CAR(&cons) = NULL;
    PARSER_CDR(&cons) = NULL;
    return parser_write_sexpr_array(cons);
}

static SExpr *parser_write_cons_atom(Token atom)
{
    SExpr *sexpr = parser_write_cons();
    PARSER_CAR(sexpr) = parser_write_atom(atom);
    return sexpr;
}

static SExpr *parser_write_cons_rule(SExpr *(*rule)())
{
    SExpr *sexpr = parser_write_cons();
    if ((PARSER_CAR(sexpr) = rule()) == NULL)
        return NULL;
    return sexpr;
}

/* Parser -- construct the s-expression(s) */

static SExpr *parser_parse_formals()
{
    // Rule: variable / "(" variable * ")"
    SExpr *sexpr, *previous, *current;

    switch (parser.this.type)
    {
    case TOKEN_SYMBOL: // First case: variable
        sexpr = parser_write_cons_atom(parser.this);
        PARSER_CDR(sexpr) = parser_write_null();
        return sexpr;

    case TOKEN_LEFT_PAREN: // Second case: "(" variable * ")"
        parser_advance();  // skip first parenthesis

        if (parser.this.type == TOKEN_RIGHT_PAREN)
        {
            sexpr = parser_write_null();
            parser_advance();
            return sexpr;
        }

        if (parser.this.type != TOKEN_SYMBOL)
            return parser_failed("Invalid formals syntax. Expected symbol.");
        sexpr = parser_write_cons_atom(parser.this);

        for (previous = sexpr; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
        {
            if (parser.this.type != TOKEN_SYMBOL)
                return parser_failed("Invalid formals syntax. Expected symbol.");
            current = parser_write_cons_atom(parser.this);
            PARSER_CDR(previous) = current;
        }

        PARSER_CDR(previous) = parser_write_null();
        parser_advance(); // skip trailing parenthesis

        return sexpr;

    default:
        return parser_failed("Expected formals.");
    }
}

static SExpr *parser_parse_body()
{
    // Rule: definition* expression+
    SExpr *sexpr, *previous, *current;
    sexpr = NULL;

    // Parse definition*
    for (previous = sexpr; parser_is_definition(); previous = current)
    {
        if ((current = parser_write_cons_rule(parser_parse_definition)) == NULL)
            return NULL;

        (sexpr == NULL) ? (sexpr = current)
                        : (PARSER_CDR(previous) = current);
    }

    // Parse expression
    if ((current = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    (sexpr == NULL) ? (sexpr = current)
                    : (PARSER_CDR(previous) = current);

    // Parse expression*
    for (previous = current; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if ((current = parser_write_cons_rule(parser_parse_expression)) == NULL)
            return NULL;

        PARSER_CDR(previous) = current;
    }

    PARSER_CDR(previous) = parser_write_null();
    // Don't skip the trailing parenthesis

    return sexpr;
}

static SExpr *parser_parse_list()
{
    // Rule: "(" datum* ")" / "(" datum + "." datum ")" / abbreviation
    SExpr *list, *previous, *current;
    list = NULL;

    if (parser.this.type != TOKEN_LEFT_PAREN)
        return parser_failed("Expected list.");
    parser_advance(); // Skip the first parenthesis

    // Parse null
    if (parser.this.type == TOKEN_RIGHT_PAREN)
    {
        list = parser_write_null();
        parser_advance();
        return list;
    }

    for (previous = list; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if ((current = parser_write_cons_rule(parser_parse_datum)) == NULL)
            return NULL;

        if (list == NULL)
            list = previous = current;
        else
            PARSER_CDR(previous) = current;

        if (parser.this.type == TOKEN_DOT)
        {
            parser_advance(); // Skip the dot
            if ((current = parser_parse_datum()) == NULL)
                return NULL;

            if (parser.this.type != TOKEN_RIGHT_PAREN)
                return parser_failed("Invalid list syntax. Expected ')'.");

            parser_advance(); // Skip the trailing parethesis
            PARSER_CDR(previous) = current;
            return list;
        }
    }

    PARSER_CDR(previous) = parser_write_null();
    parser_advance(); // skip trailing parenthesis

    return list;
}

static SExpr *parser_parse_binding()
{
    // Rule: "(" identifier expression ")"
    SExpr *id, *expr;

    if (parser.this.type != TOKEN_LEFT_PAREN)
        return parser_failed("Expected binding.");
    parser_advance(); // Skip the first parenthesis

    if (parser.this.type != TOKEN_SYMBOL)
        return parser_failed("Invalid define syntax. Expected symbol.");
    id = parser_write_cons_atom(parser.this);

    if ((expr = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Expected ')'.");

    PARSER_CDR(id) = expr;
    PARSER_CDR(expr) = parser_write_null();

    parser_advance(); // Skip the trailing parenthesis

    return id;
}

static SExpr *parser_parse_bindings()
{
    // Rule: "(" binding* ")"

    SExpr *bindings, *previous, *current;
    bindings = NULL;

    if (parser.this.type != TOKEN_LEFT_PAREN)
        return parser_failed("Expected binding specs.");
    parser_advance(); // Skip the first parenthesis

    for (previous = NULL; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if ((current = parser_write_cons_rule(parser_parse_binding)) == NULL)
            return NULL;

        (previous == NULL) ? (bindings = current)
                           : (PARSER_CDR(previous) = current);
    }
    (previous == NULL) ? (bindings = parser_write_null())
                       : (PARSER_CDR(previous) = parser_write_null());

    parser_advance(); // skip trailing parenthesis

    return bindings;
}

static SExpr *parser_parse_datum()
{
    // Rule: constant / variable / list / vector
    switch (parser.this.type)
    {
    case TOKEN_NUMBER:
    case TOKEN_SYMBOL:
    case TOKEN_STRING:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
    case TOKEN_DEFINE:
    case TOKEN_QUOTE:
    case TOKEN_LAMBDA:
    case TOKEN_IF:
    case TOKEN_SET:
    case TOKEN_CALL_CC:
        return parser_write_atom(parser.this);

    case TOKEN_LEFT_PAREN:
        return parser_parse_list();
    // case TOKEN_VECTOR:
    default:
        return parser_failed("Expected datum.");
    }
}

static SExpr *parser_parse_define()
{
    // Rule: "(" "define" symbol expression ")"
    SExpr *define, *symbol, *expr;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_DEFINE)
        return parser_failed("Invalid define syntax. Expected 'define'.");
    define = parser_write_cons_atom(parser.this);

    if (parser.this.type != TOKEN_SYMBOL)
        return parser_failed("Invalid define syntax. Expected symbol.");
    symbol = parser_write_cons_atom(parser.this);

    if ((expr = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid define syntax. Expected ')'.");

    PARSER_CDR(define) = symbol;
    PARSER_CDR(symbol) = expr;
    PARSER_CDR(expr) = parser_write_null();
    parser_advance(); // skip trailing parenthesis

    return define;
}

static SExpr *parser_parse_definition()
{
    TokenType token_type = parser.this.type;

    if (token_type == TOKEN_LEFT_PAREN)
    {
        switch (parser.lookahead.type)
        {
        case TOKEN_DEFINE:
            return parser_parse_define();
        }
    }

    return parser_failed("Invalid definition syntax.");
}

static SExpr *parser_parse_quote()
{
    // Rule: "(" "quote" datum ")"
    SExpr *quote, *datum;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_QUOTE)
        return parser_failed("Invalid expression syntax. Expected 'quote'.");
    quote = parser_write_cons_atom(parser.this);

    if ((datum = parser_write_cons_rule(parser_parse_datum)) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid quote syntax. Expected ')'.");

    PARSER_CDR(quote) = datum;
    PARSER_CDR(datum) = parser_write_null();
    parser_advance(); // skip trailing parenthesis

    return quote;
}

static SExpr *parser_parse_lambda()
{
    // Rule: "(" "lambda" formals body ")"
    SExpr *lambda, *formals, *body;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_LAMBDA)
        return parser_failed("Invalid expression syntax. Expected 'lambda'.");
    lambda = parser_write_cons_atom(parser.this);

    if ((formals = parser_write_cons_rule(parser_parse_formals)) == NULL)
        return NULL;

    if ((body = parser_parse_body()) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid lambda syntax. Expected ')'.");

    PARSER_CDR(lambda) = formals;
    PARSER_CDR(formals) = body;
    parser_advance(); // skip trailing parenthesis

    return lambda;
}

static SExpr *parser_parse_let()
{
    // Rule: "(" "let" "(" binding_spec* ")" body ")"
    SExpr *let, *bindings, *body;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_LET)
        return parser_failed("Invalid expression syntax. Expected 'lambda'.");
    let = parser_write_cons_atom(parser.this);

    if ((bindings = parser_write_cons_rule(parser_parse_bindings)) == NULL)
        return NULL;

    if ((body = parser_parse_body()) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid let syntax. Expected ')'.");

    PARSER_CDR(let) = bindings;
    PARSER_CDR(bindings) = body;
    parser_advance(); // skip trailing parenthesis

    return let;
}

static SExpr *parser_parse_begin()
{
    // Rule: "(" "let" "(" binding_spec* ")" body ")"
    SExpr *begin, *exprs, *previous, *current;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_BEGIN)
        return parser_failed("Invalid expression syntax. Expected 'lambda'.");
    begin = parser_write_cons_atom(parser.this);

    // Parse expression
    if ((exprs = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    // Parse expression*
    for (previous = exprs; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if ((current = parser_write_cons_rule(parser_parse_expression)) == NULL)
            return NULL;

        PARSER_CDR(previous) = current;
    }
    PARSER_CDR(previous) = parser_write_null();

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid let syntax. Expected ')'.");

    PARSER_CDR(begin) = exprs;
    parser_advance(); // skip trailing parenthesis

    return begin;
}

static SExpr *parser_parse_if()
{
    // Rule: "(" "if" expression expression expression ")"
    SExpr *iff, *expr1, *expr2, *expr3;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_IF)
        return parser_failed("Invalid expression syntax. Expected 'if'.");
    iff = parser_write_cons_atom(parser.this);

    if ((expr1 = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    if ((expr2 = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    if ((expr3 = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid if syntax. Expected ')'.");

    PARSER_CDR(iff) = expr1;
    PARSER_CDR(expr1) = expr2;
    PARSER_CDR(expr2) = expr3;
    PARSER_CDR(expr3) = parser_write_null();
    parser_advance(); // skip trailing parenthesis

    return iff;
}

static SExpr *parser_parse_set()
{
    // Rule: (" "set!" variable expression ")"
    SExpr *set, *symbol, *expr;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_SET)
        return parser_failed("Expected 'set!'.");
    set = parser_write_cons_atom(parser.this);

    if (parser.this.type != TOKEN_SYMBOL)
        return parser_failed("Expected symbol.");
    symbol = parser_write_cons_atom(parser.this);

    if ((expr = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid set! syntax. Expected ')'.");

    PARSER_CDR(set) = symbol;
    PARSER_CDR(symbol) = expr;
    PARSER_CDR(expr) = parser_write_null();
    parser_advance(); // skip trailing parenthesis

    return set;
}

static SExpr *parser_parse_call_cc()
{
    // Rule: (" "call/cc" expression ")"
    SExpr *call_cc, *expr;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_CALL_CC)
        return parser_failed("Expected 'call/cc'.");
    call_cc = parser_write_cons_atom(parser.this);

    if ((expr = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid call/cc syntax. Expected ')'.");

    PARSER_CDR(call_cc) = expr;
    PARSER_CDR(expr) = parser_write_null();
    parser_advance(); // skip trailing parenthesis

    return call_cc;
}

static SExpr *parser_parse_application()
{
    // Rule: "(" expression expression* ")"
    SExpr *expr, *previous, *current;
    parser_advance(); // skip first parenthesis

    if ((expr = parser_write_cons_rule(parser_parse_expression)) == NULL)
        return NULL;

    for (previous = expr; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if ((current = parser_write_cons_rule(parser_parse_expression)) == NULL)
            return NULL;
        PARSER_CDR(previous) = current;
    }

    PARSER_CDR(previous) = parser_write_null();
    parser_advance(); // skip trailing parenthesis

    return expr;
}

static SExpr *parser_parse_expression()
{
    TokenType token_type = parser.this.type;
    SExpr *sexpr;

    switch (token_type)
    {
    case TOKEN_LEFT_PAREN:
        switch (parser.lookahead.type)
        {
        case TOKEN_QUOTE:
            return parser_parse_quote();
        case TOKEN_LAMBDA:
            return parser_parse_lambda();
        case TOKEN_LET:
            return parser_parse_let();
        case TOKEN_BEGIN:
            return parser_parse_begin();
        case TOKEN_IF:
            return parser_parse_if();
        case TOKEN_SET:
            return parser_parse_set();
        case TOKEN_CALL_CC:
            return parser_parse_call_cc();
        case TOKEN_SYMBOL:
        default:
            return parser_parse_application();
        }

    // Atoms.
    case TOKEN_NUMBER:
    case TOKEN_SYMBOL:
    case TOKEN_STRING:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
        return parser_write_atom(parser.this);

    default:
        return parser_failed("Expected expression.");
    }
}

static SExpr *parser_parse_form()
{
    if (parser_is_definition())
        return parser_parse_definition();
    else
        return parser_parse_expression();
}

void parser_reset_parser()
{
    Token token = {
        .type = TOKEN_EOF,
        .start = NULL,
        .length = 0,
        .line = 0,
    };
    parser.error_token = token;

    parser.sexpr_array.capacity = 0;
    parser.sexpr_array.count = 0;
    parser.sexpr_array.sexprs = NULL;
}

void parser_init_parser(const char *source)
{
    scanner_init_scanner(source);
    parser_reset_parser();

    parser.this = parser.error_token;
    parser.lookahead = parser.error_token;

    // Load the "this" and "lookahead" tokens
    parser_advance();
    parser_advance();
}

CompileResult parser_parse(SExpr **sexpr)
{
    parser_reset_parser();

    if (parser.this.type == TOKEN_EOF)
    {
        *sexpr = NULL;
        return COMPILER_EOF;
    }

    *sexpr = parser_parse_form();

    if (*sexpr == NULL)
    {
        if (PARSER_IS_EOF(*sexpr, parser_get_error_token()))
        {
            printf("here\n");
            return COMPILER_EOF;
        }

        return COMPILER_COMPILE_ERROR;
    }
    return COMPILER_OK;
}

void parser_free_sexpr()
{
    MEMORY_FREE_ARRAY(SExpr, parser.sexpr_array.sexprs, 0);
}