#ifndef _SCANNER_H
#define _SCANNER_H

typedef enum
{
    // Single-character tokens.
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,

    // Literals.
    TOKEN_SYMBOL,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_TRUE,
    TOKEN_NULL,

    // Core.
    TOKEN_QUOTE,
    TOKEN_LAMBDA,
    TOKEN_IF,
    TOKEN_SET,
    TOKEN_CALL_CC,

    // Single character primitives.
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SLASH,
    TOKEN_STAR,

    // One or two character primitives.
    TOKEN_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,

    // Multi-character primitives.
    TOKEN_APPEND,
    TOKEN_APPLY,
    TOKEN_BOX,
    TOKEN_CAR,
    TOKEN_CDR,
    TOKEN_CONS,
    TOKEN_EQ_Q,
    TOKEN_ERROR,
    TOKEN_INTEGER_Q,
    TOKEN_LENGTH,
    TOKEN_LIST,
    TOKEN_LIST_Q,
    TOKEN_MAKE_VECTOR,
    TOKEN_MAP,
    TOKEN_NULL_Q,
    TOKEN_PAIR_Q,
    TOKEN_SET_BOX,
    TOKEN_SET_CAR,
    TOKEN_SET_CDR,
    TOKEN_STRING_Q,
    TOKEN_SYMBOL_Q,
    TOKEN_UNBOX,
    TOKEN_VECTOR_LENGTH,
    TOKEN_VECTOR_REF,
    TOKEN_VECTOR_SET,

    // Syntactic Extensions.
    // ...

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

void initScanner(const char *source);
Token scanToken();

#endif