#ifndef camel_stack_h
#define camel_stack_h

#include "common.h"
#include "value.h"

typedef struct {
    Value* values;
    Value* stackTop;
    int capacity;
} Stack;

void init_stack(Stack* stack);
void free_stack(Stack* stack);
void stack_push(Stack* stack, Value value);
Value stack_pop(Stack* stack);
Value stack_peek(Stack* stack, int distance);
void stack_reset(Stack* stack);

#endif
