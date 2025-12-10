#include "parser.h"
#include "error.h"

void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->hadError = false;
    parser->panicMode = false;
}

static void advance(Parser* parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = scan_token(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;

        error_at(parser->current.line, parser->current.start);
    }
}

// Stub parser - will be fully implemented later
ASTNode* parse(Parser* parser) {
    advance(parser);
    
    // Simple stub: just return a program node
    ASTNode* program = ast_create(AST_PROGRAM, 1, NULL);
    
    return program;
}
