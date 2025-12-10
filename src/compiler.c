#include "compiler.h"
#include "opcode.h"
#include "error.h"

void init_compiler(Compiler* compiler, Chunk* chunk) {
    compiler->chunk = chunk;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
}

static void emit_byte(Compiler* compiler, uint8_t byte, int line) {
    write_chunk(compiler->chunk, byte, line);
}

static void emit_return(Compiler* compiler, int line) {
    emit_byte(compiler, OP_RETURN, line);
}

// Stub compiler - will be fully implemented later
bool compile(Compiler* compiler, ASTNode* ast) {
    if (ast == NULL) {
        error("No AST to compile");
        return false;
    }

    // Stub: just emit a return for now
    emit_return(compiler, 1);
    return !had_error();
}
