#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "types.h"

const char *error_to_string(ErrorCode err) {
    switch(err) {
        case ERR_OK: return "OK";
        case ERR_INVALID_ARG: return "Invalid argument";
        case ERR_NULL_PTR: return "Null pointer";
        case ERR_NO_MEMORY: return "Out of memory";
        case ERR_NOT_FOUND: return "Not found";
        case ERR_ALREADY_EXISTS: return "Already exists";
        case ERR_EMPTY: return "Empty structure";
        case ERR_INVALID_STATE: return "Invalid state";
        case ERR_OUT_OF_BOUNDS: return "Index out of bounds";
        default: return "Unknown error";
    }
}

void initialize(SwitchStack *stack) {
    stack->top = -1;  
}

// Function to check if the stack is empty
bool isEmpty(SwitchStack *stack) {
    return stack->top == -1;  
}

// Function to check if the stack is full
bool isFull(SwitchStack *stack) {
    return stack->top >= MAX_STACK_SIZE - 1;  
}

// Function to push an element onto the stack
void push(SwitchStack *stack, Switch *value) {
    if (isFull(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    stack->data[++stack->top] = value;
    printf("Pushed %p onto the stack\n", value);
}

Switch *pop(SwitchStack *stack) {
    if (isEmpty(stack)) {
        printf("Stack Underflow\n");
        return NULL;
    }

    Switch *popped = stack->data[stack->top];
    stack->top--;
    printf("Popped %p from the stack\n", popped);
    return popped;
}

Switch *peek(SwitchStack *stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty\n");
        return NULL;
    }
    return stack->data[stack->top];
}
