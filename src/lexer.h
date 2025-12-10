#ifndef camel_lexer_h
#define camel_lexer_h

#include "common.h"
#include "token.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Lexer;

void init_lexer(Lexer* lexer, const char* source);
Token scan_token(Lexer* lexer);

#endif
