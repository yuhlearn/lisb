#include <scanner/scanner.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
    const char *start;
    const char *current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char *source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isSymbolChar(char c)
{
    return c == '_' || c == '-' ||
           c == '/' || c == '!' || c == '?';
}

static bool isAtEnd()
{
    return *scanner.current == '\0';
}

static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

static char peek()
{
    return *scanner.current;
}

static char peekNext()
{
    if (isAtEnd())
        return '\0';
    return scanner.current[1];
}

static bool match(char expected)
{
    if (isAtEnd())
        return false;
    if (*scanner.current != expected)
        return false;
    scanner.current++;
    return true;
}

static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char *message)
{
    Token token;
    token.type = TOKEN_FAIL;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static void skipWhitespace()
{
    for (;;)
    {
        char c = peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            scanner.line++;
            advance();
            break;
        case '/':
            if (peekNext() == '/')
            {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd())
                    advance();
            }
            else
            {
                return;
            }
            break;
        default:
            return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char *rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0)
        return type;

    return TOKEN_SYMBOL;
}

static TokenType identifierType()
{

    switch (scanner.start[0])
    {
#define LEN(str) (sizeof(str) - 1)
#define PARAM(str) LEN(str), str
    case 'a': // append apply
        if (scanner.current - scanner.start > 3 &&
            scanner.start[1] == 'p' &&
            scanner.start[2] == 'p')
        {
            switch (scanner.start[3])
            {
            case 'e': // append
                return checkKeyword(4, PARAM("nd"), TOKEN_APPEND);
            case 'l': // apply
                return checkKeyword(4, PARAM("y"), TOKEN_APPLY);
            }
        }
    case 'b': // box
        return checkKeyword(1, PARAM("ox"), TOKEN_BOX);
    case 'c':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'a':
                if (scanner.current - scanner.start > 2)
                {
                    switch (scanner.start[2])
                    {
                    case 'l': // call/cc
                        return checkKeyword(3, PARAM("l/cc"), TOKEN_CALL_CC);
                    case 'r': // car
                        return checkKeyword(3, PARAM(""), TOKEN_CAR);
                    }
                }
            case 'd': // cdr
                return checkKeyword(2, PARAM("r"), TOKEN_CDR);
            case 'o': // cons
                return checkKeyword(2, PARAM("ns"), TOKEN_CONS);
            }
        }
    case 'e':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'q': // eq?
                return checkKeyword(2, PARAM("?"), TOKEN_EQ_Q);
            case 'r': // error
                return checkKeyword(2, PARAM("ror"), TOKEN_ERROR);
            }
        }
    case 'i':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'f': // if
                return checkKeyword(2, PARAM(""), TOKEN_IF);
            case 'n': // integer?
                return checkKeyword(2, PARAM("teger?"), TOKEN_INTEGER_Q);
            }
        }
    case 'l':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'a': // lambda
                return checkKeyword(2, PARAM("mbda"), TOKEN_LAMBDA);
            case 'e': // integer?
                return checkKeyword(2, PARAM("ngth"), TOKEN_LENGTH);
            case 'i':
                if (scanner.current - scanner.start == 4) // list
                    return checkKeyword(2, PARAM("st"), TOKEN_LIST);
                if (scanner.current - scanner.start == 5) // list?
                    return checkKeyword(2, PARAM("st?"), TOKEN_LIST_Q);
            }
        }
    case 'm':
        if (scanner.current - scanner.start == 3) // map
            return checkKeyword(1, PARAM("ap"), TOKEN_MAP);
        if (scanner.current - scanner.start == 11) // make-vector
            return checkKeyword(1, PARAM("ake-vector"), TOKEN_MAKE_VECTOR);
    case 'n': // null?
        return checkKeyword(1, PARAM("ull?"), TOKEN_NULL_Q);
    case 'p': // pair?
        return checkKeyword(1, PARAM("air?"), TOKEN_PAIR_Q);
    case 'q': // quote
        return checkKeyword(1, PARAM("uote"), TOKEN_QUOTE);
    case 's':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'e': // set!
                if (scanner.current - scanner.start > 3 &&
                    scanner.start[2] == 't')
                {
                    switch (scanner.start[3])
                    {
                    case '!': // set!
                        return checkKeyword(4, PARAM(""), TOKEN_SET);
                    case '-':
                        if (scanner.current - scanner.start == LEN("set-xxx!"))
                        {
                            switch (scanner.start[4])
                            {
                            case 'b': // set-box!
                                return checkKeyword(5, PARAM("ox!"), TOKEN_SET_BOX);
                            case 'c':
                                switch (scanner.start[5])
                                {
                                case 'a': // set-car!
                                    return checkKeyword(6, PARAM("r!"), TOKEN_SET_CAR);
                                case 'd': // set-cdr!
                                    return checkKeyword(6, PARAM("r!"), TOKEN_SET_CDR);
                                }
                            }
                        }
                    }
                }
            case 't': // string?
                return checkKeyword(2, PARAM("ring?"), TOKEN_STRING_Q);
            case 'y': // symbol?
                return checkKeyword(2, PARAM("mbol?"), TOKEN_SYMBOL_Q);
            }
        }
    case 'u':
        return checkKeyword(1, PARAM("nbox"), TOKEN_UNBOX);
    case 'v':
        if (scanner.current - scanner.start == LEN("vector-length")) // vector-length
            return checkKeyword(1, PARAM("ector-length"), TOKEN_VECTOR_LENGTH);
        if (scanner.current - scanner.start == LEN("vector-ref")) // vector-ref
            return checkKeyword(1, PARAM("ector-ref"), TOKEN_VECTOR_REF);
        if (scanner.current - scanner.start == LEN("vector-set!")) // vector-set!
            return checkKeyword(1, PARAM("ector-set!"), TOKEN_VECTOR_SET);
#undef LEN
#undef PARAM
    }

    return TOKEN_SYMBOL;
}

static Token symbol()
{
    while (isAlpha(peek()) || isDigit(peek()) || isSymbolChar(peek()))
        advance();

    return makeToken(identifierType());
}

static Token number()
{
    while (isDigit(peek()))
        advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext()))
    {
        // Consume the ".".
        advance();

        while (isDigit(peek()))
            advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
            scanner.line++;
        advance();
    }

    if (isAtEnd())
        return errorToken("Unterminated string.");

    // The closing quote.
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken()
{
    skipWhitespace();
    scanner.start = scanner.current;

    if (isAtEnd())
        return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c))
        return symbol();
    if (isDigit(c))
        return number();

    switch (c)
    {
    case '(':
        return makeToken(TOKEN_LEFT_PAREN);
    case ')':
        return makeToken(TOKEN_RIGHT_PAREN);
    case '.':
        return makeToken(TOKEN_DOT);
    case '-':
        return makeToken(TOKEN_MINUS);
    case '+':
        return makeToken(TOKEN_PLUS);
    case '/':
        return makeToken(TOKEN_SLASH);
    case '*':
        return makeToken(TOKEN_STAR);
    case '=':
        return makeToken(TOKEN_EQUAL);
    case '<':
        return makeToken(
            match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(
            match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '#':
        if (match('t'))
            return makeToken(TOKEN_TRUE);
        if (match('f'))
            return makeToken(TOKEN_NULL);
    case '"':
        return string();
    }

    return errorToken("Unexpected character.");
}
