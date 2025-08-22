#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"


typedef struct Scanner {
    const char* start;
    const char* current;
    int line;
} Scanner;

//The scanner being used 
Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAlphabet(char c) {
    return (c <= 'z' && c >= 'a') ||
            (c <= 'Z' && c >= 'A') ||
            c == '_'; 
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd(){
    return *scanner.current == '\0';
}

//Goes forward, and returns the char that was just consumed
//Not always necessary to process the consumec char
static char advance() {
    scanner.current++;
    return scanner.current [-1];
}

static char peek (){
    return *scanner.current;
}

static char peekNext(){
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

//REMEMBER: it goes forward if its true
static bool match (char expected){
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;

}

static Token makeToken(TokenType type){
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

static void skipWhitespace(){
    for(;;){
        char c = peek();
        switch(c) {
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
                if (peekNext() == '/') {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                else{
                    return;
                }
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type){
    if(scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0){
            return type;
    }
    
    return TOKEN_IDENTIFIER;

}

static TokenType identifierType() {
    switch (scanner.start[0]) {
        case 'd': return checkKeyword(1, 5, "efine", TOKEN_FUNCTION);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'f':
            if (scanner.current - scanner.start > 1){
                switch (scanner.start[1]){
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                }
            }
            break;
        case 'n': return checkKeyword(1, 3, "ull", TOKEN_NULL);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 't': return checkKeyword(1, 3, "rue", TOKEN_TRUE);
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
        
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isAlphabet(peek()) || isDigit(peek())) advance();
    
    return makeToken(identifierType());
}

static Token number(){
    while (isDigit(peek())) advance();

    if(peek() == '.' && isDigit(peekNext())){
        advance();
        while(isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string(){
    while (peek() != '"' && !isAtEnd()){
        if(peek() == '\n') scanner.line++;
        advance();
    }
    if(isAtEnd()) return errorToken("Unterminated string.");

    advance();
    return makeToken(TOKEN_STRING);
}


Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;

    if(isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isAlphabet(c)) return identifier();
    if (isDigit(c)) return number();
    switch (c){
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(
            match('-') ? TOKEN_DECREMENT : 
                    (match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS));
        case '+': return makeToken(
                match('+') ? TOKEN_INCREMENT : 
                    (match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS));
        case '/': return makeToken(
            match('/') ? TOKEN_SLASH_SLASH : 
                    (match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH));
        case '*': return makeToken(
            match('*') ? TOKEN_STAR_STAR : 
                    (match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR));
        //TODO: CHECK THE || and && work
        case '|': 
            if(match('|')) return makeToken(TOKEN_OR);
            break;
        case '&':
            if(match('&')) return makeToken(TOKEN_AND);
            break;
        case '!':
            return makeToken(
                match('=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string();
    }

    return errorToken("Unexpected character.");
}