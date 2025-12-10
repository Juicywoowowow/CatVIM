#include "ast.h"
#include "memory.h"

ASTNode* ast_create(ASTNodeType type, int line, void* data) {
    ASTNode* node = ALLOCATE(ASTNode, 1);
    node->type = type;
    node->line = line;
    node->data = data;
    return node;
}

void ast_free(ASTNode* node) {
    if (node == NULL) return;
    
    // Free node data based on type (simplified for now)
    if (node->data != NULL) {
        free(node->data);
    }
    
    FREE(ASTNode, node);
}
