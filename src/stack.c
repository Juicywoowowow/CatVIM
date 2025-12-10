#include "stack.h"
#include "memory.h"
#include "error.h"

void init_stack(Stack* stack) {
    stack->capacity = STACK_MAX;
    stack->values = ALLOCATE(Value, STACK_MAX);
    stack->stackTop = stack->values;
}

void free_stack(Stack* stack) {
    FREE_ARRAY(Value, stack->values, stack->capacity);
    stack->stackTop = NULL;
}

void stack_reset(Stack* stack) {
    stack->stackTop = stack->values;
}

void stack_push(Stack* stack, Value value) {
    if (stack->stackTop - stack->values >= STACK_MAX) {
        error("Stack overflow");
        return;
    }
    *stack->stackTop = value;
    stack->stackTop++;
}

Value stack_pop(Stack* stack) {
    if (stack->stackTop <= stack->values) {
        error("Stack underflow");
        return NIL_VAL;
    }
    stack->stackTop--;
    return *stack->stackTop;
}

Value stack_peek(Stack* stack, int distance) {
    return stack->stackTop[-1 - distance];
}
