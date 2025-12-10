#ifndef camel_compiler_h
#define camel_compiler_h

#include "common.h"
#include "ast.h"
#include "chunk.h"

typedef struct {
    Chunk* chunk;
    int localCount;
    int scopeDepth;
} Compiler;

void init_compiler(Compiler* compiler, Chunk* chunk);
bool compile(Compiler* compiler, ASTNode* ast);

#endif
