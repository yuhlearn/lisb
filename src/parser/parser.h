#ifndef _PARSER_H
#define _PARSER_H

#include <scanner/scanner.h>
#include <compiler/compiler.h>
#include <value/value.h>

#define PARSER_TYPE(sexpr) ((sexpr)->type)
#define PARSER_IS_ATOM(sexpr) ((sexpr)->type == SEXPR_ATOM)
#define PARSER_IS_NULL(sexpr) ((sexpr)->type == SEXPR_NULL)
#define PARSER_IS_CONS(sexpr) ((sexpr)->type == SEXPR_CONS)
#define PARSER_IS_EOF(token) ((token).type == TOKEN_EOF && \
                              (token).start == NULL &&     \
                              (token).length == 0 &&       \
                              (token).line == 0)

#define PARSER_AS_ATOM(sexpr) ((sexpr)->value.atom)
#define PARSER_AS_CONS(sexpr) ((sexpr)->value.cons)

Token parser_get_error_token();
void parser_init_parser(const char *source);
Value parser_parse();

#endif