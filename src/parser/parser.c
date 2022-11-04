#include <parser/parser.h>
#include <memory/memory.h>
#include <common/common.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    Token lookahead;
    Token this;
    Token error_token;
    Obj *objects;
} Parser;

Parser parser;

static Value parser_parse_list();
static Value parser_parse_formals();
static Value parser_parse_body();
static Value parser_parse_datum();
static Value parser_parse_define();
static Value parser_parse_definition();
static Value parser_parse_quote();
static Value parser_parse_lambda();
static Value parser_parse_if();
static Value parser_parse_set();
static Value parser_parse_call_cc();
static Value parser_parse_application();
static Value parser_parse_expression();
static Value parser_parse_form();

/* Error management */

Token parser_get_error_token()
{
    return parser.error_token;
}

static Value parser_failed_at(Token *token, const char *message)
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

    return VALUE_VOID_VAL;
}

static Value parser_failed(const char *message)
{
    return parser_failed_at(&parser.this, message);
}

/* Parser -- utility functions */

static void parser_advance()
{
    parser.this = parser.lookahead;
    parser.lookahead = scanner_scan_token();
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

/* Manage the s-expression */

static Obj *parser_allocate_object(size_t size, ObjType type)
{
    Obj *object = malloc(size);

    if (object == NULL)
        exit(1);

    object->type = type;
    object->is_marked = false;
    object->next = parser.objects;
    parser.objects = object;

    return object;
}

static void parser_free_objects()
{
    Obj *current = parser.objects;

    while (current != NULL)
    {
        Obj *next = current->next;
        free(current);
        current = next;
    }
}

static Value parser_make_bool(bool value)
{
    Value bol = VALUE_BOOL_VAL(value);
    parser_advance();
    return bol;
}

static Value parser_make_number(Token token)
{
    Value number = VALUE_NUMBER_VAL(strtod(parser.this.start, NULL));
    parser_advance();
    return number;
}

static Value parser_make_string(Token token)
{
    ObjString *string =
        (ObjString *)parser_allocate_object(sizeof(struct ObjString), OBJ_STRING);

    string->chars = token.start + 1;
    string->length = token.length = token.length - 2;

    parser_advance();

    return VALUE_OBJ_VAL(string);
}

static Value parser_make_symbol(Token token)
{
    ObjSymbol *symbol =
        (ObjSymbol *)parser_allocate_object(sizeof(ObjSymbol), OBJ_SYMBOL);

    symbol->chars = token.start;
    symbol->length = token.length;
    symbol->token = token.type;
    symbol->line = token.line;
    symbol->row = token.row;

    parser_advance();

    return VALUE_OBJ_VAL(symbol);
}

static Value parser_make_cons()
{
    Value cons = VALUE_OBJ_VAL(parser_allocate_object(sizeof(ObjCons), OBJ_CONS));

    OBJECT_CAR(cons) = VALUE_VOID_VAL;
    OBJECT_CDR(cons) = VALUE_VOID_VAL;

    return cons;
}

static Value parser_make_cons_car_symbol(Token token)
{
    Value sexpr = parser_make_cons();

    OBJECT_CAR(sexpr) = parser_make_symbol(token);

    return sexpr;
}

static Value parser_make_cons_car_rule(Value (*rule)())
{
    Value sexpr = parser_make_cons();
    Value car = rule();

    if (VALUE_IS_VOID(car))
        return car;

    OBJECT_CAR(sexpr) = car;

    return sexpr;
}

/* Parser -- construct the s-expression(s) */

static Value parser_parse_formals()
{
    // Rule: variable / "(" variable * ")"
    Value sexpr, previous, current;

    switch (parser.this.type)
    {
    case TOKEN_SYMBOL: // First case: variable
        sexpr = parser_make_cons_car_symbol(parser.this);
        OBJECT_CDR(sexpr) = VALUE_NULL_VAL;
        return sexpr;

    case TOKEN_LEFT_PAREN: // Second case: "(" variable * ")"
        parser_advance();  // skip first parenthesis

        if (parser.this.type == TOKEN_RIGHT_PAREN)
        {
            sexpr = VALUE_NULL_VAL;
            parser_advance();
            return sexpr;
        }

        if (parser.this.type != TOKEN_SYMBOL)
            return parser_failed("Invalid formals syntax. Expected symbol.");
        sexpr = parser_make_cons_car_symbol(parser.this);

        for (previous = sexpr; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
        {
            if (parser.this.type != TOKEN_SYMBOL)
                return parser_failed("Invalid formals syntax. Expected symbol.");
            current = parser_make_cons_car_symbol(parser.this);
            OBJECT_CDR(previous) = current;
        }

        OBJECT_CDR(previous) = VALUE_NULL_VAL;
        parser_advance(); // skip trailing parenthesis

        return sexpr;

    default:
        return parser_failed("Expected formals.");
    }
}

static Value parser_parse_body()
{
    // Rule: definition* expression+
    Value sexpr, previous, current;
    sexpr = VALUE_NULL_VAL;

    // Parse definition*
    for (previous = sexpr; parser_is_definition(); previous = current)
    {
        if (VALUE_IS_VOID(current = parser_make_cons_car_rule(parser_parse_definition)))
            return VALUE_VOID_VAL;

        (VALUE_IS_NULL(sexpr)) ? (sexpr = current)
                               : (OBJECT_CDR(previous) = current);
    }

    // Parse expression
    if (VALUE_IS_VOID(current = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    (VALUE_IS_NULL(sexpr)) ? (sexpr = current)
                           : (OBJECT_CDR(previous) = current);

    // Parse expression*
    for (previous = current; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if (VALUE_IS_VOID(current = parser_make_cons_car_rule(parser_parse_expression)))
            return VALUE_VOID_VAL;

        OBJECT_CDR(previous) = current;
    }

    OBJECT_CDR(previous) = VALUE_NULL_VAL;
    // Don't skip the trailing parenthesis

    return sexpr;
}

static Value parser_parse_list()
{
    // Rule: "(" datum* ")" / "(" datum + "." datum ")" / abbreviation
    Value list, previous, current;
    list = VALUE_NULL_VAL;

    if (parser.this.type != TOKEN_LEFT_PAREN)
        return parser_failed("Expected list.");
    parser_advance(); // Skip the first parenthesis

    // Parse null
    if (parser.this.type == TOKEN_RIGHT_PAREN)
    {
        list = VALUE_NULL_VAL;
        parser_advance();
        return list;
    }

    for (previous = list; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if (VALUE_IS_VOID(current = parser_make_cons_car_rule(parser_parse_datum)))
            return VALUE_VOID_VAL;

        if (VALUE_IS_NULL(list))
            list = previous = current;
        else
            OBJECT_CDR(previous) = current;

        if (parser.this.type == TOKEN_DOT)
        {
            parser_advance(); // Skip the dot
            if (VALUE_IS_VOID(current = parser_parse_datum()))
                return VALUE_VOID_VAL;

            if (parser.this.type != TOKEN_RIGHT_PAREN)
                return parser_failed("Invalid list syntax. Expected ')'.");

            parser_advance(); // Skip the trailing parethesis
            OBJECT_CDR(previous) = current;
            return list;
        }
    }

    OBJECT_CDR(previous) = VALUE_NULL_VAL;
    parser_advance(); // skip trailing parenthesis

    return list;
}

static Value parser_parse_binding()
{
    // Rule: "(" identifier expression ")"
    Value id, expr;

    if (parser.this.type != TOKEN_LEFT_PAREN)
        return parser_failed("Expected binding.");
    parser_advance(); // Skip the first parenthesis

    if (parser.this.type != TOKEN_SYMBOL)
        return parser_failed("Invalid define syntax. Expected symbol.");
    id = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(expr = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Expected ')'.");

    OBJECT_CDR(id) = expr;
    OBJECT_CDR(expr) = VALUE_NULL_VAL;

    parser_advance(); // Skip the trailing parenthesis

    return id;
}

static Value parser_parse_bindings()
{
    // Rule: "(" binding* ")"
    Value bindings, previous, current;
    bindings = VALUE_NULL_VAL;

    if (parser.this.type != TOKEN_LEFT_PAREN)
        return parser_failed("Expected binding specs.");
    parser_advance(); // Skip the first parenthesis

    for (previous = VALUE_NULL_VAL; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if (VALUE_IS_VOID(current = parser_make_cons_car_rule(parser_parse_binding)))
            return VALUE_VOID_VAL;

        VALUE_IS_NULL(previous) ? (bindings = current)
                                : (OBJECT_CDR(previous) = current);
    }
    VALUE_IS_NULL(previous) ? (bindings = VALUE_NULL_VAL)
                            : (OBJECT_CDR(previous) = VALUE_NULL_VAL);

    parser_advance(); // skip trailing parenthesis

    return bindings;
}

static Value parser_parse_datum()
{
    // Rule: constant / variable / list / vector
    switch (parser.this.type)
    {
    case TOKEN_STRING:
        return parser_make_string(parser.this);
    case TOKEN_NUMBER:
        return parser_make_number(parser.this);
    case TOKEN_TRUE:
        return parser_make_bool(true);
    case TOKEN_FALSE:
        return parser_make_bool(false);

    case TOKEN_SYMBOL:
    case TOKEN_DEFINE:
    case TOKEN_QUOTE:
    case TOKEN_LAMBDA:
    case TOKEN_IF:
    case TOKEN_SET:
    case TOKEN_CALL_CC:
        return parser_make_symbol(parser.this);

    case TOKEN_LEFT_PAREN:
        return parser_parse_list();
    // case TOKEN_VECTOR:
    default:
        return parser_failed("Expected datum.");
    }
}

static Value parser_parse_define()
{
    // Rule: "(" "define" symbol expression ")"
    Value define, symbol, expr;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_DEFINE)
        return parser_failed("Invalid define syntax. Expected 'define'.");
    define = parser_make_cons_car_symbol(parser.this);

    if (parser.this.type != TOKEN_SYMBOL)
        return parser_failed("Invalid define syntax. Expected symbol.");
    symbol = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(expr = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid define syntax. Expected ')'.");

    OBJECT_CDR(define) = symbol;
    OBJECT_CDR(symbol) = expr;
    OBJECT_CDR(expr) = VALUE_NULL_VAL;
    parser_advance(); // skip trailing parenthesis

    return define;
}

static Value parser_parse_definition()
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

static Value parser_parse_quote()
{
    // Rule: "(" "quote" datum ")"
    Value quote, datum;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_QUOTE)
        return parser_failed("Invalid expression syntax. Expected 'quote'.");
    quote = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(datum = parser_make_cons_car_rule(parser_parse_datum)))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid quote syntax. Expected ')'.");

    OBJECT_CDR(quote) = datum;
    OBJECT_CDR(datum) = VALUE_NULL_VAL;
    parser_advance(); // skip trailing parenthesis

    return quote;
}

static Value parser_parse_lambda()
{
    // Rule: "(" "lambda" formals body ")"
    Value lambda, formals, body;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_LAMBDA)
        return parser_failed("Invalid expression syntax. Expected 'lambda'.");
    lambda = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(formals = parser_make_cons_car_rule(parser_parse_formals)))
        return VALUE_VOID_VAL;

    if (VALUE_IS_VOID(body = parser_parse_body()))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid lambda syntax. Expected ')'.");

    OBJECT_CDR(lambda) = formals;
    OBJECT_CDR(formals) = body;
    parser_advance(); // skip trailing parenthesis

    return lambda;
}

static Value parser_parse_let()
{
    // Rule: "(" "let" "(" binding_spec* ")" body ")"
    Value let, bindings, body;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_LET)
        return parser_failed("Invalid expression syntax. Expected 'lambda'.");
    let = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(bindings = parser_make_cons_car_rule(parser_parse_bindings)))
        return VALUE_VOID_VAL;

    if (VALUE_IS_VOID(body = parser_parse_body()))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid let syntax. Expected ')'.");

    OBJECT_CDR(let) = bindings;
    OBJECT_CDR(bindings) = body;
    parser_advance(); // skip trailing parenthesis

    return let;
}

static Value parser_parse_begin()
{
    // Rule: "(" "let" "(" binding_spec* ")" body ")"
    Value begin, exprs, previous, current;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_BEGIN)
        return parser_failed("Invalid expression syntax. Expected 'lambda'.");
    begin = parser_make_cons_car_symbol(parser.this);

    // Parse expression
    if (VALUE_IS_VOID(exprs = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    // Parse expression*
    for (previous = exprs; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if (VALUE_IS_VOID(current = parser_make_cons_car_rule(parser_parse_expression)))
            return VALUE_VOID_VAL;

        OBJECT_CDR(previous) = current;
    }
    OBJECT_CDR(previous) = VALUE_NULL_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid let syntax. Expected ')'.");

    OBJECT_CDR(begin) = exprs;
    parser_advance(); // skip trailing parenthesis

    return begin;
}

static Value parser_parse_if()
{
    // Rule: "(" "if" expression expression expression ")"
    Value iff, expr1, expr2, expr3;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_IF)
        return parser_failed("Invalid expression syntax. Expected 'if'.");
    iff = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(expr1 = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    if (VALUE_IS_VOID(expr2 = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    if (VALUE_IS_VOID(expr3 = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid if syntax. Expected ')'.");

    OBJECT_CDR(iff) = expr1;
    OBJECT_CDR(expr1) = expr2;
    OBJECT_CDR(expr2) = expr3;
    OBJECT_CDR(expr3) = VALUE_NULL_VAL;
    parser_advance(); // skip trailing parenthesis

    return iff;
}

static Value parser_parse_set()
{
    // Rule: (" "set!" variable expression ")"
    Value set, symbol, expr;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_SET)
        return parser_failed("Expected 'set!'.");
    set = parser_make_cons_car_symbol(parser.this);

    if (parser.this.type != TOKEN_SYMBOL)
        return parser_failed("Expected symbol.");
    symbol = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(expr = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid set! syntax. Expected ')'.");

    OBJECT_CDR(set) = symbol;
    OBJECT_CDR(symbol) = expr;
    OBJECT_CDR(expr) = VALUE_NULL_VAL;
    parser_advance(); // skip trailing parenthesis

    return set;
}

static Value parser_parse_call_cc()
{
    // Rule: (" "call/cc" expression ")"
    Value call_cc, expr;
    parser_advance(); // skip first parenthesis

    if (parser.this.type != TOKEN_CALL_CC)
        return parser_failed("Expected 'call/cc'.");
    call_cc = parser_make_cons_car_symbol(parser.this);

    if (VALUE_IS_VOID(expr = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    if (parser.this.type != TOKEN_RIGHT_PAREN)
        return parser_failed("Invalid call/cc syntax. Expected ')'.");

    OBJECT_CDR(call_cc) = expr;
    OBJECT_CDR(expr) = VALUE_NULL_VAL;
    parser_advance(); // skip trailing parenthesis

    return call_cc;
}

static Value parser_parse_application()
{
    // Rule: "(" expression expression* ")"
    Value expr, previous, current;
    parser_advance(); // skip first parenthesis

    if (VALUE_IS_VOID(expr = parser_make_cons_car_rule(parser_parse_expression)))
        return VALUE_VOID_VAL;

    for (previous = expr; parser.this.type != TOKEN_RIGHT_PAREN; previous = current)
    {
        if (VALUE_IS_VOID(current = parser_make_cons_car_rule(parser_parse_expression)))
            return VALUE_VOID_VAL;

        OBJECT_CDR(previous) = current;
    }

    OBJECT_CDR(previous) = VALUE_NULL_VAL;
    parser_advance(); // skip trailing parenthesis

    return expr;
}

static Value parser_parse_expression()
{
    TokenType token_type = parser.this.type;

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
    case TOKEN_SYMBOL:
        return parser_make_symbol(parser.this);
    case TOKEN_STRING:
        return parser_make_string(parser.this);
    case TOKEN_NUMBER:
        return parser_make_number(parser.this);
    case TOKEN_TRUE:
        return parser_make_bool(true);
    case TOKEN_FALSE:
        return parser_make_bool(false);

    default:
        return parser_failed("Expected expression.");
    }
}

static Value parser_parse_form()
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
    parser_free_objects();
    parser.objects = NULL;
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

Value parser_parse()
{
    parser_reset_parser();

    if (parser.this.type == TOKEN_EOF)
        return VALUE_VOID_VAL;

    return parser_parse_form();
}
