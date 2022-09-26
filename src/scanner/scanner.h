#ifndef _SCANNER_H
#define _SCANNER_H

typedef enum
{
    // Single-character tokens.
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_DOT,

    // Literals.
    TOKEN_NUMBER,
    TOKEN_SYMBOL,
    TOKEN_STRING,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,

    // Core definitions.
    TOKEN_DEFINE,

    // Core expressions.
    TOKEN_QUOTE,
    TOKEN_LAMBDA,
    TOKEN_IF,
    TOKEN_SET,
    TOKEN_CALL_CC,

    // Other.
    TOKEN_FAIL,
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

void scanner_init_scanner(const char *source);
Token scanner_scan_token();

#endif