#ifndef camel_ast_h
#define camel_ast_h

#include "common.h"
#include "value.h"

// Forward declarations
typedef struct ASTNode ASTNode;

typedef enum {
    AST_PROGRAM,
    AST_NAMESPACE,
    AST_VAR_DECL,
    AST_FUNC_DECL,
    AST_PARAMETER,
    AST_BLOCK,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_RETURN_STMT,
    AST_CALL_STMT,
    AST_SET_STMT,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_LITERAL,
    AST_IDENTIFIER,
} ASTNodeType;

// Base AST node
struct ASTNode {
    ASTNodeType type;
    int line;
    void* data;
};

ASTNode* ast_create(ASTNodeType type, int line, void* data);
void ast_free(ASTNode* node);

#endif
