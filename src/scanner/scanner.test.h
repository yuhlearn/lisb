#ifndef _SCANNER_TEST_H
#define _SCANNER_TEST_H

#include <scanner/scanner.h>
#include <CUnit/Basic.h>

void scannerScanTokenTest()
{
    const char *input =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_!?/ \
         1234567890 #t #f . - + / * = < <= > >=\"string\" () append \
         apply box call/cc car cdr cons eq? error if integer? \
         lambda length list list? make-vector map null? pair? quote \
         set! set-box! set-car! set-cdr! string? symbol? unbox \
         vector-length vector-ref vector-set! & \1";

    initScanner(input);

    CU_ASSERT_EQUAL(scanToken().type, TOKEN_SYMBOL);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_NUMBER);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_TRUE);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_NULL);

    CU_ASSERT_EQUAL(scanToken().type, TOKEN_DOT);

    CU_ASSERT_EQUAL(scanToken().type, TOKEN_MINUS);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_PLUS);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_SLASH);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_STAR);

    CU_ASSERT_EQUAL(scanToken().type, TOKEN_EQUAL);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_LESS);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_LESS_EQUAL);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_GREATER);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_GREATER_EQUAL);

    CU_ASSERT_EQUAL(scanToken().type, TOKEN_STRING);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_LEFT_PAREN);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_RIGHT_PAREN);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_APPEND);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_APPLY);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_BOX);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_CALL_CC);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_CAR);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_CDR);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_CONS);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_EQ_Q);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_ERROR);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_IF);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_INTEGER_Q);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_LAMBDA);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_LENGTH);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_LIST);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_LIST_Q);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_MAKE_VECTOR);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_MAP);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_NULL_Q);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_PAIR_Q);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_QUOTE);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_SET);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_SET_BOX);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_SET_CAR);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_SET_CDR);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_STRING_Q);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_SYMBOL_Q);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_UNBOX);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_VECTOR_LENGTH);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_VECTOR_REF);
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_VECTOR_SET);

    CU_ASSERT_EQUAL(scanToken().type, TOKEN_FAIL);

    CU_ASSERT_EQUAL(scanToken().type, TOKEN_EOF);
    // check that it's not reading beyond the end of the string
    CU_ASSERT_EQUAL(scanToken().type, TOKEN_EOF);
}

#endif