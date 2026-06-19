#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "lexer.h"

enum NodeType {
    NODE_NUMBER,
    NODE_BOOL,
    NODE_BINARY_OP,
    NODE_BLOCK,
    NODE_VARIABLE,
    NODE_FUNCTION_CALL,
    NODE_ASSIGN,
    NODE_UNARY_OP,
    NODE_COMPARISON,
    NODE_IF,
    NODE_WHILE
};

enum ParserErrorCode {
    PARSE_OK,
    SYNTAX_ERROR,
    VALUE_ERROR,
    NAME_ERROR,

};

struct ErrorParser {
    enum ParserErrorCode error_type;
    char *msg;
    int arund;
};

struct Arena;

typedef struct ASTNode {
    enum NodeType type;
    union {
        struct {
            double value;
        } number;
        struct {
            bool value;
        } boolean;
        struct {
            char *name;
        } variable;
        struct {
            char *name;
            struct ASTNode *arguments;
            size_t count;
        } call;
        struct {
            enum TokenType op;
            struct ASTNode *right;
        } unary;
        struct {
            enum TokenType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;
        struct {
            enum TokenType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } comparison;
        struct {
            char *name;
            struct ASTNode *value;
        } assign;
        struct {
            struct ASTNode **children;
            size_t count;
            size_t capacity;
        } block;
        struct {
            struct ASTNode *condition;
            struct ASTNode *if_branch;
            struct ASTNode *else_branch;
        } if_statement;
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_statement;
    };
    //   struct ASTNode *next;
    struct Arena *arena;
} ASTNode;

ASTNode *parse_tokens(Token *tokens, size_t count);

void print_ast(ASTNode *root);
void free_ast(ASTNode *root);
