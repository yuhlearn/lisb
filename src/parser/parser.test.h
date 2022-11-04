#ifndef _PARSER_TEST_H
#define _PARSER_TEST_H

#include <CUnit/Basic.h>
#include <scanner/scanner.h>
#include <parser/parser.h>
#include <value/value.h>
#include <object/object.h>

void parser_parse_constant_test_1()
{
    char *input;
    Value sexpr;

    input = "1";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE(VALUE_IS_NUMBER(sexpr));
}

void parser_parse_constant_test_2()
{
    char *input;
    Value sexpr;

    input = "\"hej hej\"";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE(OBJECT_IS_STRING(sexpr));
}

void parser_parse_constant_test_3()
{
    char *input;
    Value sexpr;

    input = "#t";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE(VALUE_IS_BOOL(sexpr));
}

void parser_parse_constant_test_4()
{
    char *input;
    Value sexpr;

    input = "#f";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE(VALUE_IS_BOOL(sexpr));
}

void parser_parse_symbol_test_1()
{
    char *input;
    Value sexpr;

    input = "-.!$%&*+-./:<=>?@^_~abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE(OBJECT_IS_SYMBOL(sexpr));
}

void parser_parse_symbol_test_2()
{
    char *input;
    Value sexpr;

    input = "lambda";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));

    input = ".";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
}

void parser_parse_quote_test_1()
{
    char *input;
    Value sexpr;

    input = "(quote 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    // Check that the first element is "quote"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_QUOTE);

    // Check that the second element is "1"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_quote_test_2()
{
    char *input;
    Value sexpr;

    input = "(quote x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    // Check that the second element is "x"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CDAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CDAR(sexpr))->token, TOKEN_SYMBOL);

    // Check that the last element is null
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_quote_test_3()
{
    char *input;
    Value sexpr;

    input = "(quote)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
}

void parser_parse_quote_list_test_1()
{
    char *input;
    Value sexpr;

    input = "(quote ())";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));

    // Check that the second and last element is null
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDAR(sexpr)));
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_quote_list_test_2()
{
    char *input;
    Value sexpr;

    input = "(quote (1))";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));

    // Check that the second element is a list
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE(OBJECT_IS_CONS(OBJECT_CDAR(sexpr)));

    // Check that it contains one and only one number
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAAR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_quote_list_test_3()
{
    char *input;
    Value sexpr;

    input = "(quote (1 2))";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    // Check that the second element is a list
    // and that it contains two and only two numbers
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDAR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAAR(sexpr)));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDADR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDADAR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDADDR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_quote_list_test_4()
{
    char *input;
    Value sexpr;

    input = "(quote (1 . 2))";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    // Check that the second element is a list
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDAR(sexpr)));

    // Check that it contains two and only two numbers
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAAR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDADR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_quote_list_test_5()
{
    char *input;
    Value sexpr;

    input = "(quote (1 . ()))";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    // Check that the second element is a list
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDAR(sexpr)));

    // Check that it contains one and only one number
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAAR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_lambda_test()
{
    char *input;
    Value sexpr;

    input = "(lambda () 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    // Check that the first element is "lambda"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_LAMBDA);

    // Check that the formals element is a list
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NULL(OBJECT_CDAR(sexpr)));

    // Check that the body element is a list
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDDR(sexpr)));
    CU_ASSERT_TRUE(VALUE_IS_NUMBER(OBJECT_CDDAR(sexpr)));

    // Check that the last element is null
    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDDR(sexpr)));
}

void parser_parse_lambda_formals_test_1()
{
    Value sexpr;
    char *input;

    input = "(lambda () 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    // Check that the formals element contains null
    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDAR(sexpr)));
}

void parser_parse_lambda_formals_test_2()
{
    Value sexpr, formals;
    char *input;

    input = "(lambda x 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    formals = OBJECT_CDAR(sexpr);

    // Check that the formals element contains only "x"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(formals));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(formals)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(formals))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDR(formals)));
}

void parser_parse_lambda_formals_test_3()
{
    Value sexpr, formals;
    char *input;

    input = "(lambda (x) 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    formals = OBJECT_CDAR(sexpr);

    // Check that the formals element contains only "x"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(formals));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(formals)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(formals))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDR(formals)));
}

void parser_parse_lambda_formals_test_4()
{
    Value sexpr, formals;
    char *input;

    input = "(lambda (x y) 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    formals = OBJECT_CDAR(sexpr);

    // Check that the formals element contains only "x" and "y"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(formals));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(formals)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(formals))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(formals)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CDAR(formals)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CDAR(formals))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(formals)));
}

void parser_parse_lambda_body_test_1()
{
    Value sexpr, body;
    char *input;

    input = "(lambda () 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    body = OBJECT_CDDR(sexpr);

    // Check that the body element contains only "1"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(body));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CAR(body)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDR(body)));
}

void parser_parse_lambda_body_test_2()
{
    Value sexpr, body;
    char *input;

    input = "(lambda () 1 2)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    body = OBJECT_CDDR(sexpr);

    // Check that the formals element contains only "1" and "2"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(body));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CAR(body)));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(body)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAR(body)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(body)));
}

void parser_parse_lambda_body_test_3()
{
    Value sexpr, body;
    char *input;

    input = "(lambda () (define x 1) x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    body = OBJECT_CDDR(sexpr);

    // Check that the formals element contains only definition and "x"
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(body));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CAR(body)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAAR(body)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAAR(body))->token, TOKEN_DEFINE);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(body)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CDAR(body)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CDAR(body))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(body)));
}

void parser_parse_lambda_fail_test_1()
{
    Value sexpr, body;
    char *input;

    input = "(lambda () (define x 1))";
    parser_init_parser(input);
    sexpr = parser_parse();

    // Check that it returns NULL, since there are no expressions in body
    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_lambda_fail_test_2()
{
    Value sexpr, body;
    char *input;

    input = "(lambda x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    // Check that it returns NULL, since there are no expressions in body
    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_lambda_fail_test_3()
{
    Value sexpr, body;
    char *input;

    input = "(lambda (1) (define x 1))";
    parser_init_parser(input);
    sexpr = parser_parse();

    // Check that it returns NULL, the forms are malformed
    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_NUMBER);
}

void parser_parse_lambda_fail_test_4()
{
    Value sexpr, body;
    char *input;

    input = "(lambda (define x 1) x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);

    // Check that it returns NULL, since there are no formals
    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
}

void parser_parse_let_test_1()
{
    Value sexpr, bindings, body;
    char *input;

    input = "(let () 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_LET);

    // Check that the bindings are null
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NULL(OBJECT_CDAR(sexpr)));
}

void parser_parse_let_test_2()
{
    Value sexpr, bindings, body;
    char *input;

    input = "(let ((x 1)) x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_LET);

    // Check that the bindings constains bindings
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDAR(sexpr)));
    bindings = OBJECT_CDAR(sexpr);

    // Check that the bindings constains (x 1)
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CAR(bindings)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAAR(bindings)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAAR(bindings))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CADR(bindings)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CADAR(bindings)));

    CU_ASSERT_TRUE_FATAL(VALUE_IS_NULL(OBJECT_CADDR(bindings)));

    CU_ASSERT_TRUE_FATAL(VALUE_IS_NULL(OBJECT_CDR(bindings)));
}

void parser_parse_let_test_3()
{
    Value sexpr, bindings, body;
    char *input;

    input = "(let ((x 1) (y 2)) x y)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_LET);

    // Check that the bindings constains bindings
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDAR(sexpr)));
    bindings = OBJECT_CDAR(sexpr);

    // Check that the bindings constains (x 1)
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CAR(bindings)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAAR(bindings)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAAR(bindings))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CADR(bindings)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CADAR(bindings)));

    CU_ASSERT_TRUE_FATAL(VALUE_IS_NULL(OBJECT_CADDR(bindings)));

    // Check that the bindings constains (y 2)
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDAR(bindings)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CDAAR(bindings)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CDAAR(bindings))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDADR(bindings)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDADAR(bindings)));

    CU_ASSERT_TRUE_FATAL(VALUE_IS_NULL(OBJECT_CDADDR(bindings)));

    CU_ASSERT_TRUE_FATAL(VALUE_IS_NULL(OBJECT_CDDR(bindings)));
}

void parser_parse_begin_test_1()
{
    Value sexpr, body;
    char *input;

    input = "(begin 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_BEGIN);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));

    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_begin_test_2()
{
    Value sexpr, body;
    char *input;

    input = "(begin 1 2)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_BEGIN);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAR(sexpr)));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDDAR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDDR(sexpr)));
}

void parser_parse_begin_fail_test_1()
{
    Value sexpr, body;
    char *input;

    input = "(begin)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_begin_fail_test_2()
{
    Value sexpr, body;
    char *input;

    input = "(begin (define x 1) x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);
}

void parser_parse_if_test()
{
    Value sexpr, body;
    char *input;

    input = "(if #t 1 2)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_IF);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_BOOL(OBJECT_CDAR(sexpr)));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDDAR(sexpr)));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDDDAR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDDDR(sexpr)));
}

void parser_parse_set_test_1()
{
    Value sexpr, body;
    char *input;

    input = "(set! x 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_SET);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CDAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CDAR(sexpr))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDDAR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDDR(sexpr)));
}

void parser_parse_set_test_2()
{
    Value sexpr, body;
    char *input;

    input = "(set! #t 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_TRUE);
}

void parser_parse_set_test_3()
{
    Value sexpr, body;
    char *input;

    input = "(set! x (define y 1))";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_DEFINE);
}

void parser_parse_call_cc_test_1()
{
    Value sexpr, body;
    char *input;

    input = "(call/cc x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_CALL_CC);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CDAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CDAR(sexpr))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_call_cc_test_2()
{
    Value sexpr, body;
    char *input;

    input = "(call/cc x y)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_SYMBOL);
}

void parser_parse_call_cc_test_3()
{
    Value sexpr, body;
    char *input;

    input = "(call/cc set!)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_SET);
}

void parser_parse_call_cc_test_4()
{
    Value sexpr, body;
    char *input;

    input = "(call/cc)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

void parser_parse_application_test_1()
{
    Value sexpr, body;
    char *input;

    input = "(x 1 #f)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAR(sexpr)));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_BOOL(OBJECT_CDDAR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDDR(sexpr)));
}

void parser_parse_application_test_2()
{
    Value sexpr, body;
    char *input;

    input = "(x 1)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(OBJECT_CDR(sexpr)));
    CU_ASSERT_TRUE_FATAL(VALUE_IS_NUMBER(OBJECT_CDAR(sexpr)));

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDDR(sexpr)));
}

void parser_parse_application_test_3()
{
    Value sexpr, body;
    char *input;

    input = "(x)";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_FALSE_FATAL(VALUE_IS_VOID(sexpr));

    CU_ASSERT_TRUE_FATAL(OBJECT_IS_CONS(sexpr));
    CU_ASSERT_TRUE_FATAL(OBJECT_IS_SYMBOL(OBJECT_CAR(sexpr)));
    CU_ASSERT_EQUAL(OBJECT_AS_SYMBOL(OBJECT_CAR(sexpr))->token, TOKEN_SYMBOL);

    CU_ASSERT_TRUE(VALUE_IS_NULL(OBJECT_CDR(sexpr)));
}

void parser_parse_application_test_4()
{
    Value sexpr, body;
    char *input;

    input = "()";
    parser_init_parser(input);
    sexpr = parser_parse();

    CU_ASSERT_TRUE_FATAL(VALUE_IS_VOID(sexpr));
    CU_ASSERT_EQUAL(parser_get_error_token().type, TOKEN_RIGHT_PAREN);
}

#endif