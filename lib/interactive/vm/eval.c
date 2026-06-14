#include "eval.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/lexer.h"
#include "../parser/parser.h"

static Value make_none(void) {
    Value v;
    v.type = VALUE_NONE;
    v.data.number_value = 0;
    return v;
}

static Value make_number(double number) {
    Value v;
    v.type = VALUE_NUMBER;
    v.data.number_value = number;
    return v;
}

static Value make_bool(int truthy) {
    Value v;
    v.type = VALUE_BOOL;
    v.data.bool_value = truthy ? 1 : 0;
    return v;
}

static double value_to_number(Value value) {
    switch(value.type) {
    case VALUE_NUMBER:
        return value.data.number_value;
    case VALUE_BOOL:
        return value.data.bool_value ? 1.0 : 0.0;
    default:
        return 0.0;
    }
}

static int value_is_truthy(Value value) {
    switch(value.type) {
    case VALUE_BOOL:
        return value.data.bool_value != 0;
    case VALUE_NUMBER:
        return value.data.number_value != 0.0;
    default:
        return 0;
    }
}

static void print_value(Value value) {
    switch(value.type) {
    case VALUE_NUMBER:
        printf("%f", value.data.number_value);
        break;
    case VALUE_BOOL:
        printf(value.data.bool_value ? "True" : "False");
        break;
    default:
        printf("None");
        break;
    }
}

static Value eval_binary(ASTNode *node);
static void eval_statement(ASTNode *node);
static Value eval_expression(ASTNode *node);
static Value eval_function_call(ASTNode *node);

Value vm_print(ASTNode *arg) {
    if(arg) {
        Value result = eval_expression(arg);
        print_value(result);
        printf("\n");
        return make_none();
    }
    printf("Expected something to print\n");
    return make_none();
}

Value vm_foo(ASTNode *arg) {
    (void) arg;
    printf("FOO\n");
    return make_none();
}

Value vm_exit(ASTNode *arg) {
    if(arg) {
        Value result = eval_expression(arg);
        exit((int) value_to_number(result));
    }
    exit(0);
}

static Variable variables[250] = {0};
static size_t variable_count = 0;

static void set_variable(const char *name, Value value) {
    for(size_t i = 0; i < variable_count; i++) {
        if(strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }

    if(variable_count >= 250) {
        printf("Too many variables\n");
        return;
    }

    variables[variable_count].name = strdup(name);
    variables[variable_count].value = value;
    variable_count++;
}

static Value get_variable(char *name) {
    for(size_t i = 0; i < variable_count; i++) {
        if(strcmp(variables[i].name, name) == 0) return variables[i].value;
    }

    printf("Undefined variable: %s\n", name);
    return make_none();
}

void eval_ast(ASTNode *root) {
    if(!root || root->type != NODE_BLOCK) { return; }
    for(size_t i = 0; i < root->block.count; i++) {
        eval_statement(root->block.children[i]);
    }
    printf("\n\n\n");
    for(size_t i = 0; i < variable_count; i++) {
        if(!variables[i].name) break;
        printf("variable %s, value ", variables[i].name);
        print_value(variables[i].value);
        printf("\n");
    }
}

void eval_block(ASTNode *node) {
    if(node->type != NODE_BLOCK) return;
    for(size_t i = 0; i < node->block.count; i++) {
        eval_statement(node->block.children[i]);
    }
}

void eval_statement(ASTNode *node) {
    if(!node) return;

    switch(node->type) {
    case NODE_ASSIGN: {
        Value value = eval_expression(node->assign.value);
        set_variable(node->assign.name, value);
        break;
    }
    case NODE_IF: {
        Value condition = eval_expression(node->if_statement.condition);
        if(value_is_truthy(condition)) {
            eval_block(node->if_statement.if_branch);
        } else if(node->if_statement.else_branch) {
            eval_block(node->if_statement.else_branch);
        }
        break;
    }
    case NODE_WHILE: {
        while(value_is_truthy(eval_expression(node->while_statement.condition))) {
            eval_block(node->while_statement.body);
        }
        break;
    }
    default: {
        (void) eval_expression(node);
        break;
    }
    }
}

Value eval_expression(ASTNode *node) {
    if(!node) return make_none();
    switch(node->type) {
    case NODE_BINARY_OP:
        return eval_binary(node);
    case NODE_NUMBER:
        return make_number(node->number.value);
    case NODE_BOOL:
        return make_bool(node->boolean.value);
    case NODE_VARIABLE:
        return get_variable(node->variable.name);
    case NODE_FUNCTION_CALL:
        return eval_function_call(node);
    case NODE_UNARY_OP: {
        Value value = eval_expression(node->unary.right);
        switch(node->unary.op) {
        case TOKEN_MINUS:
            return make_number(-value_to_number(value));
        case TOKEN_PLUS:
            return make_number(value_to_number(value));
        default:
            return make_none();
        }
    }
    default:
        return make_none();
    }
}

Value eval_binary(ASTNode *node) {
    Value left = eval_expression(node->binary.left);
    Value right = eval_expression(node->binary.right);
    switch(node->binary.op) {
    case TOKEN_PLUS:
        return make_number(value_to_number(left) + value_to_number(right));
    case TOKEN_MINUS:
        return make_number(value_to_number(left) - value_to_number(right));
    case TOKEN_STAR:
        return make_number(value_to_number(left) * value_to_number(right));
    case TOKEN_SLASH:
        if(value_to_number(right) == 0.0) return make_none();
        return make_number(value_to_number(left) / value_to_number(right));
    case TOKEN_LESS:
        return make_bool(value_to_number(left) < value_to_number(right));
    case TOKEN_LESS_EQUAL:
        return make_bool(value_to_number(left) <= value_to_number(right));
    case TOKEN_GREATER:
        return make_bool(value_to_number(left) > value_to_number(right));
    case TOKEN_GREATER_EQUAL:
        return make_bool(value_to_number(left) >= value_to_number(right));
    case TOKEN_EQUAL:
        return make_bool(value_to_number(left) == value_to_number(right));
    case TOKEN_NOT_EQUAL:
        return make_bool(value_to_number(left) != value_to_number(right));
    default:
        printf("Unknown operand\n");
        return make_none();
    }
}

struct call_options funcs[] = {
    {"print", 1, vm_print}, {"foo", 0, vm_foo}, {"exit", 0, vm_exit}};

Value eval_function_call(ASTNode *node) {
    for(size_t i = 0; i < sizeof(funcs) / sizeof(funcs[0]); i++) {
        if(strcmp(funcs[i].name, node->call.name) == 0)
            return funcs[i].func(node->call.arguments);
    }
    return make_none();
}
