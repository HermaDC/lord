//types.c
#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "types.h"

const char *error_to_string(ErrorCode err) {
    switch(err) {
        case ERR_OK: return "OK";
        case ERR_GENERAL: return "General error";
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

ErrorCode init_system(System *sys, size_t initial_capacity){
    if (!sys) return ERR_NULL_PTR;
    if (initial_capacity == 0) return ERR_INVALID_ARG;

    sys->count = 0;
    sys->buffer = initial_capacity;
    sys->array = malloc(initial_capacity * sizeof(Track));

    if (!sys->array) {
        sys->buffer = 0;
        return ERR_NO_MEMORY;
    }

    return ERR_OK;

}
void free_system(System *sys){
    int count = sys->count;
    for (int i = 0; i < count; i++) {
        free(sys->array[i].sensors);
    }
    free(sys->array);
    sys->array = NULL;
    sys->count = 0;
    sys->buffer = 0;
}

void initialize_stack(SwitchStack *stack) {
    stack->top = -1;  
}

// Function to check if the stack is empty
bool is_empty(SwitchStack *stack) {
    return stack->top == -1;  
}

// Function to check if the stack is full
bool is_full(SwitchStack *stack) {
    return stack->top >= MAX_STACK_SIZE - 1;  
}

// Function to push an element onto the stack
void push(SwitchStack *stack, int value) {
    if (is_full(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    stack->data[++stack->top] = value;
    printf("Pushed %d onto the stack\n", value);
}

int pop(SwitchStack *stack) {
    if (is_empty(stack)) {
        printf("Stack Underflow\n");
        return -1;
    }

    int popped = stack->data[stack->top];
    stack->top--;
    printf("Popped %d from the stack\n", popped);
    return popped;
}

int peek(SwitchStack *stack) {
    if (is_empty(stack)) {
        printf("Stack is empty\n");
        return -1;
    }
    return stack->data[stack->top];
}
