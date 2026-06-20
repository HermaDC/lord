#include "eval.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "builtins.h"
#include "utils.h"

static Value eval_binary(VM *vm, ASTNode *node);
static void eval_statement(VM *vm, ASTNode *node);
static Value eval_expression(VM *vm, ASTNode *node);
static Value eval_function_call(VM *vm, ASTNode *node);

inline VM make_VM(void) {
    VM vm;
    vm.var_capacity = 32;
    vm.variables = malloc(32 * sizeof(Variable));
    vm.var_count = 0;
    vm.error_code = 0;
    vm.should_exit = 0;
    return vm;
}

inline void destroy_VM(VM *vm) {
    for(size_t i = 0; i < vm->var_count; i++) {
        free(vm->variables[i].name);
        vm->variables[i].name = NULL;
    }
    vm->var_count = 0;
    vm->var_capacity = 0;
    free(vm->variables);
    vm->variables = NULL;
}

static void set_variable(VM *vm, const char *name, Value value) {
    for(size_t i = 0; i < vm->var_count; i++) {
        Variable tmp_var = vm->variables[i];
        if(strcmp(tmp_var.name, name) == 0) {
            vm->variables[i].value = value;
            return;
        }
    }

    if(vm->var_count >= 256) {
        printf("Too many variables\n");
        return;
    }

    vm->variables[vm->var_count].name = strdup(name);
    vm->variables[vm->var_count].value = value;
    vm->var_count++;
}

static Value get_variable(VM *vm, char *name) {
    for(size_t i = 0; i < vm->var_count; i++) {
        if(strcmp(vm->variables[i].name, name) == 0) return vm->variables[i].value;
    }

    printf("Undefined variable: %s\n", name);
    return make_none();
}

void eval_ast(VM *vm, ASTNode *root) {
    if(!root || root->type != NODE_BLOCK) { return; }
    for(size_t i = 0; i < root->block.count; i++) {
        if(vm->should_exit) return;
        eval_statement(vm, root->block.children[i]);
    }
}

void eval_block(VM *vm, ASTNode *node) {
    if(node->type != NODE_BLOCK) return;
    for(size_t i = 0; i < node->block.count; i++) {
        eval_statement(vm, node->block.children[i]);
    }
}

void eval_statement(VM *vm, ASTNode *node) {
    if(!node) return;

    switch(node->type) {
    case NODE_ASSIGN: {
        Value value = eval_expression(vm, node->assign.value);
        set_variable(vm, node->assign.name, value);
        break;
    }
    case NODE_IF: {
        Value condition = eval_expression(vm, node->if_statement.condition);
        if(value_is_truthy(condition)) {
            eval_block(vm, node->if_statement.if_branch);
        } else if(node->if_statement.else_branch) {
            eval_block(vm, node->if_statement.else_branch);
        }
        break;
    }
    case NODE_WHILE: {
        while(value_is_truthy(eval_expression(vm, node->while_statement.condition))) {
            eval_block(vm, node->while_statement.body);
        }
        break;
    }
    default: {
        (void) eval_expression(vm, node);
        break;
    }
    }
}

Value eval_expression(VM *vm, ASTNode *node) {
    if(!node) return make_none();
    switch(node->type) {
    case NODE_BINARY_OP:
        return eval_binary(vm, node);
    case NODE_NUMBER:
        return make_number(node->number.value);
    case NODE_BOOL:
        return make_bool(node->boolean.value);
    case NODE_VARIABLE:
        return get_variable(vm, node->variable.name);
    case NODE_FUNCTION_CALL:
        return eval_function_call(vm, node);
    case NODE_UNARY_OP: {
        Value value = eval_expression(vm, node->unary.right);
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

Value eval_binary(VM *vm, ASTNode *node) {
    Value left = eval_expression(vm, node->binary.left);
    Value right = eval_expression(vm, node->binary.right);
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

Value eval_function_call(VM *vm, ASTNode *node) {
    Value arg = make_none();
    if(node->call.arguments) { arg = eval_expression(vm, node->call.arguments); }

    for(size_t i = 0; i < count_call_options; i++) {
        if(strcmp(funcs[i].name, node->call.name) == 0) return funcs[i].func(vm, arg);
    }
    printf("Function not found\n");
    vm->should_exit = 1;
    vm->error_code = 2;
    return make_none();
}
