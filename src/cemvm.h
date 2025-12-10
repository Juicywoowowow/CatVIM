#ifndef camel_cemvm_h
#define camel_cemvm_h

#include "common.h"
#include "chunk.h"
#include "value.h"
#include "table.h"
#include "stack.h"

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Stack stack;
    Table globals;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void init_vm(VM* vm);
void free_vm(VM* vm);
InterpretResult interpret(VM* vm, const char* source);
InterpretResult run_chunk(VM* vm, Chunk* chunk);

#endif
