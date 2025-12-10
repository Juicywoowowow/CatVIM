#include "cemvm.h"
#include "opcode.h"
#include "debug.h"
#include "error.h"
#include "parser.h"
#include "compiler.h"
#include "ast.h"
#include <stdarg.h>

void init_vm(VM* vm) {
    init_stack(&vm->stack);
    init_table(&vm->globals);
    vm->chunk = NULL;
    vm->ip = NULL;
}

void free_vm(VM* vm) {
    free_stack(&vm->stack);
    free_table(&vm->globals);
}

static void runtime_error(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm->ip - vm->chunk->code - 1;
    int line = vm->chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    stack_reset(&vm->stack);
}

static bool is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run(VM* vm) {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])
#define READ_SHORT() \
    (vm->ip += 2, (uint16_t)((vm->ip[-2] << 8) | vm->ip[-1]))

#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_I64(stack_peek(&vm->stack, 0)) || !IS_I64(stack_peek(&vm->stack, 1))) { \
            runtime_error(vm, "Operands must be numbers"); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        int64_t b = AS_I64(stack_pop(&vm->stack)); \
        int64_t a = AS_I64(stack_pop(&vm->stack)); \
        stack_push(&vm->stack, valueType(a op b)); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm->stack.values; slot < vm->stack.stackTop; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");
        disassemble_instruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
#endif

        uint8_t instruction = READ_BYTE();
        switch (instruction) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                stack_push(&vm->stack, constant);
                break;
            }
            case OP_NIL: stack_push(&vm->stack, NIL_VAL); break;
            case OP_TRUE: stack_push(&vm->stack, BOOL_VAL(true)); break;
            case OP_FALSE: stack_push(&vm->stack, BOOL_VAL(false)); break;
            case OP_POP: stack_pop(&vm->stack); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                stack_push(&vm->stack, vm->stack.values[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm->stack.values[slot] = stack_peek(&vm->stack, 0);
                break;
            }
            case OP_GET_GLOBAL: {
                Value name = READ_CONSTANT();
                Value value;
                if (!table_get(&vm->globals, AS_STRING(name), &value)) {
                    runtime_error(vm, "Undefined variable '%s'", AS_STRING(name));
                    return INTERPRET_RUNTIME_ERROR;
                }
                stack_push(&vm->stack, value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                Value name = READ_CONSTANT();
                table_set(&vm->globals, AS_STRING(name), stack_peek(&vm->stack, 0));
                stack_pop(&vm->stack);
                break;
            }
            case OP_SET_GLOBAL: {
                Value name = READ_CONSTANT();
                if (table_set(&vm->globals, AS_STRING(name), stack_peek(&vm->stack, 0))) {
                    table_delete(&vm->globals, AS_STRING(name));
                    runtime_error(vm, "Undefined variable '%s'", AS_STRING(name));
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = stack_pop(&vm->stack);
                Value a = stack_pop(&vm->stack);
                stack_push(&vm->stack, BOOL_VAL(values_equal(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD:      BINARY_OP(I64_VAL, +); break;
            case OP_SUBTRACT: BINARY_OP(I64_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(I64_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(I64_VAL, /); break;
            case OP_NOT:
                stack_push(&vm->stack, BOOL_VAL(is_falsey(stack_pop(&vm->stack))));
                break;
            case OP_NEGATE:
                if (!IS_I64(stack_peek(&vm->stack, 0))) {
                    runtime_error(vm, "Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                stack_push(&vm->stack, I64_VAL(-AS_I64(stack_pop(&vm->stack))));
                break;
            case OP_PRINT: {
                print_value(stack_pop(&vm->stack));
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                vm->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (is_falsey(stack_peek(&vm->stack, 0))) vm->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                vm->ip -= offset;
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef BINARY_OP
}

InterpretResult run_chunk(VM* vm, Chunk* chunk) {
    vm->chunk = chunk;
    vm->ip = vm->chunk->code;
    return run(vm);
}

InterpretResult interpret(VM* vm, const char* source) {
    // Lexer
    Lexer lexer;
    init_lexer(&lexer, source);
    
    // Parser
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse(&parser);
    
    if (parser.hadError || ast == NULL) {
        ast_free(ast);
        return INTERPRET_COMPILE_ERROR;
    }
    
    // Compiler
    Chunk chunk;
    init_chunk(&chunk);
    
    Compiler compiler;
    init_compiler(&compiler, &chunk);
    
    if (!compile(&compiler, ast)) {
        free_chunk(&chunk);
        ast_free(ast);
        return INTERPRET_COMPILE_ERROR;
    }
    
    ast_free(ast);
    
    // Execute
    InterpretResult result = run_chunk(vm, &chunk);
    
    free_chunk(&chunk);
    return result;
}
