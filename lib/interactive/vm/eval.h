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

typedef struct {
    Variable *variables;
    size_t var_count;
    size_t var_capacity;
    int should_exit;
    int error_code;
} VM;

typedef Value (*vm_func_call)(VM *vm, Value arg);
struct call_options {
    const char *name;
    int min_args;
    vm_func_call func;
};

VM make_VM(void);
void eval_ast(VM *vm, ASTNode *root);
void destroy_VM(VM *vm);
