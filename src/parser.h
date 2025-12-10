#ifndef camel_parser_h
#define camel_parser_h

#include "common.h"
#include "token.h"
#include "ast.h"
#include "lexer.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

void init_parser(Parser* parser, Lexer* lexer);
ASTNode* parse(Parser* parser);

#endif
