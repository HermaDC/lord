#include "eval.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "builtins.h"
#include "utils.h"

static Value eval_binary(ASTNode *node);
static void eval_statement(ASTNode *node);
static Value eval_expression(ASTNode *node);
static Value eval_function_call(ASTNode *node);

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

Value eval_function_call(ASTNode *node) {
    Value arg = make_none();
    if(node->call.arguments) { arg = eval_expression(node->call.arguments); }

    for(size_t i = 0; i < count_call_options; i++) {
        if(strcmp(funcs[i].name, node->call.name) == 0) return funcs[i].func(arg);
    }
    return make_none();
}
