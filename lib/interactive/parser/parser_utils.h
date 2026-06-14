#pragma once

#include "parser.h"
#include <stddef.h>

typedef struct Arena Arena;

Arena *arena_create(size_t chunk_size);
void arena_destroy(Arena *arena);
void *arena_alloc(Arena *arena, size_t size);
char *arena_strdup(Arena *arena, const char *source);

ASTNode *make_node(Arena *arena, enum NodeType type);
ASTNode *make_number(Arena *arena, double value);
ASTNode *make_false(Arena *arena);
ASTNode *make_true(Arena *arena);
ASTNode *make_function_call(Arena *arena, char *name, ASTNode *arg);
ASTNode *make_binary(Arena *arena, enum TokenType op, ASTNode *left, ASTNode *right);
ASTNode *make_block(Arena *arena);
ASTNode *make_variable(Arena *arena, const char *name);
ASTNode *make_assign(Arena *arena, const char *name, ASTNode *value);
ASTNode *make_unary(Arena *arena, enum TokenType type, ASTNode *right);
ASTNode *make_if(Arena *arena, ASTNode *conditon, ASTNode *if_branch, ASTNode *else_branch);
ASTNode *make_while(Arena *arena, ASTNode *condition, ASTNode *body);
void push_to_block(Arena *arena, ASTNode *parent, ASTNode *child);
