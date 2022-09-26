#ifndef _SCANNER_TEST_H
#define _SCANNER_TEST_H

#include <CUnit/Basic.h>
#include <scanner/scanner.h>

void scanner_scan_token_test()
{
    const char *input =
        "-.!$%&*+-./:<=>?@^_~abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 \
         -1234567890 #t #f . \"string\" ( ) call/cc define if lambda quote set! #";

    scanner_init_scanner(input);

    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_SYMBOL);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_NUMBER);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_TRUE);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_FALSE);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_DOT);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_STRING);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_LEFT_PAREN);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_RIGHT_PAREN);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_CALL_CC);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_DEFINE);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_IF);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_LAMBDA);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_QUOTE);
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_SET);

    // # is not valid input in this context
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_FAIL);

    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_EOF);
    // check that it's not reading beyond the end of the string
    CU_ASSERT_EQUAL(scanner_scan_token().type, TOKEN_EOF);
}

#endif