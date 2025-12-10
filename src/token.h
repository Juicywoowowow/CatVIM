#ifndef camel_token_h
#define camel_token_h

#include "common.h"

typedef enum {
    // Single-character tokens
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COLON, TOKEN_COMMA, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SLASH, TOKEN_STAR,
    
    // One or two character tokens
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    
    // Literals
    TOKEN_IDENTIFIER, TOKEN_STRING,
    TOKEN_I8, TOKEN_I16, TOKEN_I32, TOKEN_I64,
    TOKEN_U8, TOKEN_U16, TOKEN_U32, TOKEN_U64,
    TOKEN_F32, TOKEN_F64,
    
    // Keywords
    TOKEN_AND, TOKEN_BUILTIN, TOKEN_CALL, TOKEN_DEFINE,
    TOKEN_ELSE, TOKEN_END, TOKEN_FALSE, TOKEN_FOR,
    TOKEN_FROM, TOKEN_FUNCTION, TOKEN_IF, TOKEN_NAMESPACE,
    TOKEN_NIL, TOKEN_NOT, TOKEN_OR, TOKEN_PARAMETER,
    TOKEN_PRIVATE, TOKEN_PUBLIC, TOKEN_RETURN, TOKEN_RETURN_TYPE,
    TOKEN_SET, TOKEN_TO, TOKEN_TRUE, TOKEN_VARIABLE,
    TOKEN_WHILE,
    
    // Type keywords
    TOKEN_TYPE_I8, TOKEN_TYPE_I16, TOKEN_TYPE_I32, TOKEN_TYPE_I64,
    TOKEN_TYPE_U8, TOKEN_TYPE_U16, TOKEN_TYPE_U32, TOKEN_TYPE_U64,
    TOKEN_TYPE_F32, TOKEN_TYPE_F64,
    TOKEN_TYPE_STRING, TOKEN_TYPE_BOOL, TOKEN_TYPE_VOID,
    
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

void init_token(Token* token, TokenType type, const char* start, int length, int line);
const char* token_type_name(TokenType type);

#endif
