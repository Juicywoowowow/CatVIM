#include "ast_print.h"
#include <stdio.h>

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void ast_print(ASTNode* node, int indent) {
    if (node == NULL) return;

    print_indent(indent);
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_NAMESPACE:
            printf("Namespace\n");
            break;
        case AST_VAR_DECL:
            printf("VarDecl\n");
            break;
        case AST_FUNC_DECL:
            printf("FuncDecl\n");
            break;
        case AST_LITERAL:
            printf("Literal\n");
            break;
        case AST_IDENTIFIER:
            printf("Identifier\n");
            break;
        default:
            printf("Unknown AST node\n");
            break;
    }
}
