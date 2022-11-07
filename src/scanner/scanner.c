#include <scanner/scanner.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
    const char *start;
    const char *current;
    int line;
    const char *line_begin;
} Scanner;

Scanner scanner;

void scanner_init_scanner(const char *source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
    scanner.line_begin = source;
}

static bool scanner_is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

static bool scanner_is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool scanner_is_extended_char(char c)
{
    return (c >= 36 && c <= 38) ||                       // $ % &
           (c >= 42 && c <= 47 && c != 44) ||            // * + - . /
           (c >= 58 && c <= 64 && c != 59) ||            // : < = > ? @
           c == '!' || c == '^' || c == '_' || c == '~'; // ! ^ _ ~
}

static bool scanner_is_at_end()
{
    return *scanner.current == '\0';
}

static char scanner_advance()
{
    scanner.current++;
    return scanner.current[-1];
}

static char scanner_peek()
{
    return *scanner.current;
}

static char scanner_peek_next()
{
    if (scanner_is_at_end())
        return '\0';
    return scanner.current[1];
}

static bool scanner_match(char expected)
{
    if (scanner_is_at_end())
        return false;
    if (*scanner.current != expected)
        return false;
    scanner.current++;
    return true;
}

static Token scanner_make_token(TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    token.row = scanner.current - scanner.line_begin;
    return token;
}

static Token scanner_error_token(const char *message)
{
    Token token;
    token.type = TOKEN_FAIL;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    token.row = scanner.current - scanner.line_begin;
    return token;
}

static void scanner_skip_whitespace()
{
    for (;;)
    {
        char c = scanner_peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            scanner_advance();
            break;
        case '\n':
            scanner.line++;
            scanner.line_begin = scanner.current;
            scanner_advance();
            break;
        case ';':
            // A comment goes until the end of the line.
            while (scanner_peek() != '\n' && !scanner_is_at_end())
                scanner_advance();
            break;
        default:
            return;
        }
    }
}

static TokenType scanner_check_keyword(int start, int length, const char *rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0)
        return type;

    return TOKEN_SYMBOL;
}

static TokenType scanner_identifier_type()
{

    switch (scanner.start[0])
    {
#define SCANNER_LENGTH(str) (sizeof(str) - 1)
#define SCANNER_ARGS(str1, str2) SCANNER_LENGTH(str1), SCANNER_LENGTH(str2), str2
    case '.':
        return scanner_check_keyword(SCANNER_ARGS(".", ""), TOKEN_DOT);
    case 'b':
        return scanner_check_keyword(SCANNER_ARGS("b", "egin"), TOKEN_BEGIN);
    case 'c':
        return scanner_check_keyword(SCANNER_ARGS("c", "all/cc"), TOKEN_CALL_CC);
    case 'd':
        return scanner_check_keyword(SCANNER_ARGS("d", "efine"), TOKEN_DEFINE);
    case 'i':
        return scanner_check_keyword(SCANNER_ARGS("i", "f"), TOKEN_IF);
    case 'l':
        if (scanner.current - scanner.start > 1)
            switch (scanner.start[1])
            {
            case 'a':
                return scanner_check_keyword(SCANNER_ARGS("la", "mbda"), TOKEN_LAMBDA);
            case 'e':
                return scanner_check_keyword(SCANNER_ARGS("le", "t"), TOKEN_LET);
            }
    case 'q':
        return scanner_check_keyword(SCANNER_ARGS("q", "uote"), TOKEN_QUOTE);
    case 's':
        return scanner_check_keyword(SCANNER_ARGS("s", "et!"), TOKEN_SET);
#undef SCANNER_LENGTH
#undef SCANNER_ARGS
    }

    return TOKEN_SYMBOL;
}

static Token scanner_symbol()
{
    while (scanner_is_alpha(scanner_peek()) ||
           scanner_is_digit(scanner_peek()) ||
           scanner_is_extended_char(scanner_peek()))
        scanner_advance();

    return scanner_make_token(scanner_identifier_type());
}

static Token scanner_number()
{
    while (scanner_is_digit(scanner_peek()))
        scanner_advance();

    // Look for a fractional part.
    if (scanner_peek() == '.' && scanner_is_digit(scanner_peek_next()))
    {
        // Consume the ".".
        scanner_advance();

        while (scanner_is_digit(scanner_peek()))
            scanner_advance();
    }

    return scanner_make_token(TOKEN_NUMBER);
}

static Token scanner_string()
{
    while (scanner_peek() != '"' && !scanner_is_at_end())
    {
        if (scanner_peek() == '\n')
        {
            scanner.line++;
            scanner.line_begin = scanner.current;
        }
        scanner_advance();
    }

    if (scanner_is_at_end())
        return scanner_error_token("Unterminated string.");

    // The closing quote.
    scanner_advance();
    return scanner_make_token(TOKEN_STRING);
}

Token scanner_scan_token()
{
    scanner_skip_whitespace();
    scanner.start = scanner.current;

    if (scanner_is_at_end())
        return scanner_make_token(TOKEN_EOF);

    char c = scanner_advance();
    if (scanner_is_digit(c) || (c == '-' && scanner_is_digit(scanner_peek())))
        return scanner_number();

    if (scanner_is_alpha(c) || scanner_is_extended_char(c))
        return scanner_symbol();

    switch (c)
    {
    case '(':
        return scanner_make_token(TOKEN_LEFT_PAREN);
    case ')':
        return scanner_make_token(TOKEN_RIGHT_PAREN);
    case '#':
        if (scanner_match('t'))
            return scanner_make_token(TOKEN_TRUE);
        if (scanner_match('f'))
            return scanner_make_token(TOKEN_FALSE);
        break;
    case '"':
        return scanner_string();
    }
    return scanner_error_token("Illegal character.");
}
