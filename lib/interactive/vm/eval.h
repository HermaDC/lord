#pragma once
#include "../parser/parser.h"

typedef enum {
    VALUE_NUMBER,
    VALUE_BOOL,
    VALUE_STR,
    VALUE_NONE,
} ValueType;

typedef struct {
    union {
        double number_value;
        char bool_value;
    } data;
    ValueType type;
} Value;

typedef struct {
    char *name;
    Value value;
} Variable;

typedef Value (*vm_func_call)(Value arg);
struct call_options {
    const char *name;
    int min_args;
    vm_func_call func;
};

void eval_ast(ASTNode *root);
