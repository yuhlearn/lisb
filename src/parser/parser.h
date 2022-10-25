#ifndef _PARSER_H
#define _PARSER_H

#include <scanner/scanner.h>
#include <compiler/compiler.h>

typedef enum
{
    PARSER_OK,
    PARSER_EOF,
    PARSER_ERROR,
} ParseResult;

typedef enum
{
    SEXPR_CONS,
    SEXPR_NULL,
    SEXPR_ATOM,
} SExprType;

typedef struct
{
    struct SExpr *car;
    struct SExpr *cdr;
} ConsCell;

typedef struct SExpr
{
    SExprType type;
    union
    {
        ConsCell cons;
        Token atom;
    } value;
} SExpr;

#define PARSER_CAR(sexpr) ((sexpr)->value.cons.car)
#define PARSER_CDR(sexpr) ((sexpr)->value.cons.cdr)

#define PARSER_CAAR(sexpr) (PARSER_CAR(PARSER_CAR(sexpr)))
#define PARSER_CADR(sexpr) (PARSER_CDR(PARSER_CAR(sexpr)))
#define PARSER_CDAR(sexpr) (PARSER_CAR(PARSER_CDR(sexpr)))
#define PARSER_CDDR(sexpr) (PARSER_CDR(PARSER_CDR(sexpr)))

#define PARSER_CAAAR(sexpr) (PARSER_CAR(PARSER_CAAR(sexpr)))
#define PARSER_CAADR(sexpr) (PARSER_CDR(PARSER_CAAR(sexpr)))
#define PARSER_CADAR(sexpr) (PARSER_CAR(PARSER_CADR(sexpr)))
#define PARSER_CADDR(sexpr) (PARSER_CDR(PARSER_CADR(sexpr)))
#define PARSER_CDAAR(sexpr) (PARSER_CAR(PARSER_CDAR(sexpr)))
#define PARSER_CDADR(sexpr) (PARSER_CDR(PARSER_CDAR(sexpr)))
#define PARSER_CDDAR(sexpr) (PARSER_CAR(PARSER_CDDR(sexpr)))
#define PARSER_CDDDR(sexpr) (PARSER_CDR(PARSER_CDDR(sexpr)))

#define PARSER_CAAAAR(sexpr) (PARSER_CAAR(PARSER_CAAR(sexpr)))
#define PARSER_CAADAR(sexpr) (PARSER_CDAR(PARSER_CAAR(sexpr)))
#define PARSER_CAADDR(sexpr) (PARSER_CDDR(PARSER_CAAR(sexpr)))
#define PARSER_CADAAR(sexpr) (PARSER_CAAR(PARSER_CADR(sexpr)))
#define PARSER_CADADR(sexpr) (PARSER_CADR(PARSER_CADR(sexpr)))
#define PARSER_CADDAR(sexpr) (PARSER_CDAR(PARSER_CADR(sexpr)))
#define PARSER_CADDDR(sexpr) (PARSER_CDDR(PARSER_CADR(sexpr)))
#define PARSER_CDAAAR(sexpr) (PARSER_CAAR(PARSER_CDAR(sexpr)))
#define PARSER_CDAADR(sexpr) (PARSER_CADR(PARSER_CDAR(sexpr)))
#define PARSER_CDADAR(sexpr) (PARSER_CDAR(PARSER_CDAR(sexpr)))
#define PARSER_CDADDR(sexpr) (PARSER_CDDR(PARSER_CDAR(sexpr)))
#define PARSER_CDDAAR(sexpr) (PARSER_CAAR(PARSER_CDDR(sexpr)))
#define PARSER_CDDADR(sexpr) (PARSER_CADR(PARSER_CDDR(sexpr)))
#define PARSER_CDDDAR(sexpr) (PARSER_CDAR(PARSER_CDDR(sexpr)))
#define PARSER_CDDDDR(sexpr) (PARSER_CDDR(PARSER_CDDR(sexpr)))

#define PARSER_TYPE(sexpr) ((sexpr)->type)
#define PARSER_IS_ATOM(sexpr) ((sexpr)->type == SEXPR_ATOM)
#define PARSER_IS_NULL(sexpr) ((sexpr)->type == SEXPR_NULL)
#define PARSER_IS_CONS(sexpr) ((sexpr)->type == SEXPR_CONS)
#define PARSER_IS_EOF(sexpr, token) ((sexpr) == NULL &&           \
                                     (token).type == TOKEN_EOF && \
                                     (token).start == NULL &&     \
                                     (token).length == 0 &&       \
                                     (token).line == 0)

#define PARSER_AS_ATOM(sexpr) ((sexpr)->value.atom)
#define PARSER_AS_CONS(sexpr) ((sexpr)->value.cons)

Token parser_get_error_token();
void parser_init_parser(const char *source);
ParseResult parser_parse(SExpr **sexpr);

#endif