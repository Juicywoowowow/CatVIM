#include "token.h"

void init_token(Token* token, TokenType type, const char* start, int length, int line) {
    token->type = type;
    token->start = start;
    token->length = length;
    token->line = line;
}

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_COLON: return "COLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_STAR: return "STAR";
        case TOKEN_BANG: return "BANG";
        case TOKEN_BANG_EQUAL: return "BANG_EQUAL";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_LESS: return "LESS";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_DEFINE: return "DEFINE";
        case TOKEN_NAMESPACE: return "NAMESPACE";
        case TOKEN_FUNCTION: return "FUNCTION";
        case TOKEN_VARIABLE: return "VARIABLE";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}
