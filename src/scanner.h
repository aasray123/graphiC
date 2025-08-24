#ifndef graphiC_scanner_h
#define graphiC_scanner_h

typedef enum{
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_MINUS, TOKEN_PLUS, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_INCREMENT, TOKEN_DECREMENT,
    TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL, TOKEN_SLASH_EQUAL, TOKEN_STAR_EQUAL,
    TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_DOT, 
    // One or two character tokens.
    TOKEN_NOT, TOKEN_NOT_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    // Keywords.
    TOKEN_AND, TOKEN_OR, TOKEN_ELSE,
    TOKEN_FOR, TOKEN_WHILE, TOKEN_FUNCTION, TOKEN_IF, 
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_NULL, 
    TOKEN_FALSE, TOKEN_TRUE, TOKEN_VAR, 
    TOKEN_ERROR,
    TOKEN_EOF

} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

void initScanner(const char* source);
Token scanToken();


#endif graphiC_scanner_h