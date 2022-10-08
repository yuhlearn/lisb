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

void parser_parse_constant_test_1()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "1";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_NUMBER);
}

void parser_parse_constant_test_2()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "\"hej hej\"";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_STRING);
}

void parser_parse_constant_test_3()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "#t";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_TRUE);
}

void parser_parse_constant_test_4()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "#f";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_FALSE);
}

void parser_parse_symbol_test_1()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "-.!$%&*+-./:<=>?@^_~abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE(PARSER_IS_ATOM(sexpr));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(sexpr).type, TOKEN_SYMBOL);
}

void parser_parse_symbol_test_2()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "lambda";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);

    input = ".";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
}

void parser_parse_quote_test_1()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    // Check that the first element is "quote"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_QUOTE);

    // Check that the second element is "1"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_quote_test_2()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    // Check that the second element is "x"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_SYMBOL);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_quote_test_3()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
}

void parser_parse_quote_list_test_1()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote ())";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));

    // Check that the second and last element is null
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDAR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_quote_list_test_2()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote (1))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));

    // Check that the second element is a list
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    // Check that it contains one and only one number
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(sexpr)).type, TOKEN_NUMBER);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_quote_list_test_3()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote (1 2))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    // Check that the second element is a list
    // and that it contains two and only two numbers
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDADR(sexpr)));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDADAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDADAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDADDR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_quote_list_test_4()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote (1 . 2))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    // Check that the second element is a list
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    // Check that it contains two and only two numbers
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(sexpr)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDADR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDADR(sexpr)).type, TOKEN_NUMBER);

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_quote_list_test_5()
{
    char *input;
    SExpr *sexpr;
    CompileResult result;

    input = "(quote (1 . ()))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    // Check that the second element is a list
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDAR(sexpr)));

    // Check that it contains one and only one number
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAAR(sexpr)));
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

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    // Check that the first element is "lambda"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_LAMBDA);

    // Check that the formals element is a list
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_NULL(PARSER_CDAR(sexpr)));

    // Check that the body element is a list
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDR(sexpr)));
}

void parser_parse_lambda_formals_test_1()
{
    SExpr *sexpr;
    char *input;
    CompileResult result;

    input = "(lambda () 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that the formals element contains null
    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDAR(sexpr)));
}

void parser_parse_lambda_formals_test_2()
{
    SExpr *sexpr, *formals;
    char *input;
    CompileResult result;

    input = "(lambda x 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    formals = PARSER_CDAR(sexpr);

    // Check that the formals element contains only "x"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(formals));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(formals)));
}

void parser_parse_lambda_formals_test_3()
{
    SExpr *sexpr, *formals;
    char *input;
    CompileResult result;

    input = "(lambda (x) 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    formals = PARSER_CDAR(sexpr);

    // Check that the formals element contains only "x"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(formals));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(formals)));
}

void parser_parse_lambda_formals_test_4()
{
    SExpr *sexpr, *formals;
    char *input;
    CompileResult result;

    input = "(lambda (x y) 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    formals = PARSER_CDAR(sexpr);

    // Check that the formals element contains only "x" and "y"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(formals));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(formals)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(formals)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(formals)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(formals)));
}

void parser_parse_lambda_body_test_1()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda () 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    body = PARSER_CDDR(sexpr);

    // Check that the body element contains only "1"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(body));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(body)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(body)));
}

void parser_parse_lambda_body_test_2()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda () 1 2)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    body = PARSER_CDDR(sexpr);

    // Check that the formals element contains only "1" and "2"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(body));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(body)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(body)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(body)).type, TOKEN_NUMBER);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(body)));
}

void parser_parse_lambda_body_test_3()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda () (define x 1) x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    body = PARSER_CDDR(sexpr);

    // Check that the formals element contains only definition and "x"
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(body));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CAR(body)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAAR(body)).type, TOKEN_DEFINE);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(body)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(body)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(body)).type, TOKEN_SYMBOL);
    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(body)));
}

void parser_parse_lambda_fail_test_1()
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
}

void parser_parse_lambda_fail_test_2()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that it returns NULL, since there are no expressions in body
    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_lambda_fail_test_3()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda (1) (define x 1))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    // Check that it returns NULL, the forms are malformed
    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_NUMBER);
}

void parser_parse_lambda_fail_test_4()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(lambda (define x 1) x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);

    // Check that it returns NULL, since there are no formals
    CU_ASSERT_EQUAL(sexpr, NULL);
}

void parser_parse_let_test_1()
{
    SExpr *sexpr, *bindings, *body;
    char *input;
    CompileResult result;

    input = "(let () 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_LET);

    // Check that the bindings are null
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_NULL(PARSER_CDAR(sexpr)));
}

void parser_parse_let_test_2()
{
    SExpr *sexpr, *bindings, *body;
    char *input;
    CompileResult result;

    input = "(let ((x 1)) x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_LET);

    // Check that the bindings constains bindings
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDAR(sexpr)));
    bindings = PARSER_CDAR(sexpr);

    // Check that the bindings constains (x 1)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CAR(bindings)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAAR(bindings)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAAR(bindings)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CADR(bindings)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CADAR(bindings)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CADAR(bindings)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_NULL(PARSER_CADDR(bindings)));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_NULL(PARSER_CDR(bindings)));
}

void parser_parse_let_test_3()
{
    SExpr *sexpr, *bindings, *body;
    char *input;
    CompileResult result;

    input = "(let ((x 1) (y 2)) x y)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_LET);

    // Check that the bindings constains bindings
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDAR(sexpr)));
    bindings = PARSER_CDAR(sexpr);

    // Check that the bindings constains (x 1)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CAR(bindings)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAAR(bindings)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAAR(bindings)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CADR(bindings)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CADAR(bindings)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CADAR(bindings)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_NULL(PARSER_CADDR(bindings)));

    // Check that the bindings constains (y 2)
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDAR(bindings)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAAR(bindings)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAAR(bindings)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDADR(bindings)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDADAR(bindings)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDADAR(bindings)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_NULL(PARSER_CDADDR(bindings)));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_NULL(PARSER_CDDR(bindings)));
}

void parser_parse_begin_test_1()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(begin 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_BEGIN);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_begin_test_2()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(begin 1 2)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);
    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_BEGIN);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDDR(sexpr)));

    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDR(sexpr)));
}

void parser_parse_begin_fail_test_1()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(begin)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_begin_fail_test_2()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(begin (define x 1) x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);
}

void parser_parse_if_test()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(if #t 1 2)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_IF);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_TRUE);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDDR(sexpr)));
}

void parser_parse_set_test_1()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(set! x 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SET);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDR(sexpr)));
}

void parser_parse_set_test_2()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(set! #t 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_TRUE);
}

void parser_parse_set_test_3()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(set! x (define y 1))";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);
}

void parser_parse_call_cc_test_1()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(call/cc x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_CALL_CC);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_call_cc_test_2()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(call/cc x y)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_SYMBOL);
}

void parser_parse_call_cc_test_3()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(call/cc set!)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_SET);
}

void parser_parse_call_cc_test_4()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(call/cc)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_application_test_1()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(x 1 #f)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDDAR(sexpr)).type, TOKEN_FALSE);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDDR(sexpr)));
}

void parser_parse_application_test_2()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(x 1)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(PARSER_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CDAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CDAR(sexpr)).type, TOKEN_NUMBER);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDDR(sexpr)));
}

void parser_parse_application_test_3()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "(x)";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_NOT_EQUAL_FATAL(sexpr, NULL);

    CU_ASSERT_TRUE_FATAL(PARSER_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(PARSER_IS_ATOM(PARSER_CAR(sexpr)));
    CU_ASSERT_EQUAL(PARSER_AS_ATOM(PARSER_CAR(sexpr)).type, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(PARSER_IS_NULL(PARSER_CDR(sexpr)));
}

void parser_parse_application_test_4()
{
    SExpr *sexpr, *body;
    char *input;
    CompileResult result;

    input = "()";
    parser_init_parser(input);
    result = parser_parse(&sexpr);

    CU_ASSERT_EQUAL(sexpr, NULL);
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

#endif