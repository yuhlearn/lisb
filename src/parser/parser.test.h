#ifndef _PARSER_TEST_H
#define _PARSER_TEST_H

#include <CUnit/Basic.h>
#include <scanner/scanner.h>
#include <parser/parser.h>

void parser_parse_empty_test()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    Token error_token = parser_get_error_token();

    CU_ASSERT_TRUE(PARSER_IS_EOF(sexpr, error_token));

    input = "x";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_FALSE(PARSER_IS_EOF(sexpr, error_token));

    result = parser_parse(&sexpr);
    error_token = parser_get_error_token();

    CU_ASSERT_TRUE(PARSER_IS_EOF(sexpr, error_token));

    input = "1 (define x 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_FALSE(PARSER_IS_EOF(sexpr, error_token));

    result = parser_parse(&sexpr);
    error_token = parser_get_error_token();

    CU_ASSERT_FALSE(PARSER_IS_EOF(sexpr, error_token));

    result = parser_parse(&sexpr);
    error_token = parser_get_error_token();

    CU_ASSERT_TRUE(PARSER_IS_EOF(sexpr, error_token));
}

void parser_parse_constant_test()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "1";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_NUMBER);

    input = "\"hej hej\"";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_STRING);

    input = "#t";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_TRUE);

    input = "#f";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_FALSE);
}

void parser_parse_symbol_test()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "-.!$%&*+-./:<=>?@^_~abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_SYMBOL);

    input = "lambda";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);

    input = ".";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
}

void parser_parse_quote_test()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    // Check that the first element is "quote"
    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_QUOTE);

    // Check that the second element is "1"
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(quote x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    // Check that the second element is "x"
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_SYMBOL);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(quote)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
}

void parser_parse_quote_list_test()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote ())";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that the second and last element is null
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDAR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(quote (1))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that the second element is a list
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    // Check that it contains one and only one number
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(sexpr)).type, TOKEN_NUMBER);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(quote (1 2))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that the second element is a list
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    // Check that it contains two and only two numbers
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(sexpr)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDADAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDADAR(sexpr)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDADDR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(quote (1 . 2))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that the second element is a list
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    // Check that it contains two and only two numbers
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(sexpr)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDADR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDADR(sexpr)).type, TOKEN_NUMBER);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(quote (1 . ()))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that the second element is a list
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    // Check that it contains one and only one number
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(sexpr)).type, TOKEN_NUMBER);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_lambda_test()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(lambda () 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    // Check that the first element is "lambda"
    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_LAMBDA);

    // Check that the formals element is a list
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    // Check that the body element is a list
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDDAR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDR(sexpr)));
}

void parser_parse_lambda_formals_test()
{
    SExpr *sexpr, *formals;
    char *input;
    CompileResult result;

    input = "(lambda () 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    formals = PARSER_CDAR(sexpr);

    // Check that the formals element contains null
    CU_ASSERT_TRUE(PARSER_IS_NULL(formals));

    input = "(lambda x 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    formals = PARSER_CDAR(sexpr);

    // Check that the formals element contains only "x"
    CU_ASSERT_TRUE(PARSER_IS_CONS(formals));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(formals)));

    input = "(lambda (x) 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    formals = PARSER_CDAR(sexpr);

    // Check that the formals element contains only "x"
    CU_ASSERT_TRUE(PARSER_IS_CONS(formals));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(formals)));

    input = "(lambda (x y) 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    formals = PARSER_CDAR(sexpr);

    // Check that the formals element contains only "x" and "y"
    CU_ASSERT_TRUE(PARSER_IS_CONS(formals));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(formals)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(formals)));
}

void parser_parse_lambda_body_test()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda () 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    body = PARSER_CDDAR(sexpr);

    // Check that the body element contains only "1"
    CU_ASSERT_TRUE(PARSER_IS_CONS(body));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(body)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(body)));

    input = "(lambda () 1 2)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    body = PARSER_CDDAR(sexpr);

    // Check that the formals element contains only "1" and "2"
    CU_ASSERT_TRUE(PARSER_IS_CONS(body));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(body)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(body)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(body)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(body)));

    input = "(lambda () (define x 1) x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);
    body = PARSER_CDDAR(sexpr);

    // Check that the formals element contains only definition and "x"
    CU_ASSERT_TRUE(PARSER_IS_CONS(body));
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CAR(body)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAAR(body)).type, TOKEN_DEFINE);
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(body)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(body)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(body)));
}

void parser_parse_lambda_fail_test()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda () (define x 1))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that it returns NULL, since there are no expressions in body
    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);

    input = "(lambda x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that it returns NULL, since there are no expressions in body
    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);

    input = "(lambda (1) (define x 1))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that it returns NULL, the forms are malformed
    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_NUMBER);

    input = "(lambda (define x 1) x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);

    // Check that it returns NULL, since there are no formals
    CU_ASSERT_EQUAL(sexpr, NULL);
}

void parser_parse_if_test()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(if #t 1 2)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_IF);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_TRUE);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDDDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDDR(sexpr)));
}

void parser_parse_set_test()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(set! x 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SET);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDR(sexpr)));

    input = "(set! #t 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_TRUE);

    input = "(set! x (define y 1))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);
}

void parser_parse_call_cc_test()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(call/cc x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_CALL_CC);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(call/cc x y)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_SYMBOL);

    input = "(call/cc set!)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_SET);

    input = "(call/cc)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_application_test()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(x 1 #f)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDAR(sexpr)).type, TOKEN_FALSE);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDR(sexpr)));

    input = "(x 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));

    input = "(x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL(sexpr, NULL);

    CU_ASSERT_TRUE(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(sexpr)));

    input = "()";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

#endif