#include "parser_utils.h"

#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

#define ARENA_DEFAULT_CHUNK 4096

typedef struct ArenaChunk {
    struct ArenaChunk *next;
    size_t capacity;
    size_t used;
    unsigned char data[];
} ArenaChunk;

struct Arena {
    ArenaChunk *head;
    size_t chunk_size;
};

static size_t align_size(size_t size) {
    const size_t alignment = sizeof(void *);
    return (size + alignment - 1) & ~(alignment - 1);
}

Arena *arena_create(size_t chunk_size) {
    if(chunk_size == 0) chunk_size = ARENA_DEFAULT_CHUNK;

    Arena *arena = malloc(sizeof(Arena));
    if(!arena) return NULL;

    arena->chunk_size = chunk_size;
    arena->head = NULL;
    return arena;
}

void arena_destroy(Arena *arena) {
    if(!arena) return;

    ArenaChunk *chunk = arena->head;
    while(chunk) {
        ArenaChunk *next = chunk->next;
        free(chunk);
        chunk = next;
    }
    free(arena);
}

void *arena_alloc(Arena *arena, size_t size) {
    if(!arena || size == 0) return NULL;

    size = align_size(size);
    ArenaChunk *chunk = arena->head;
    if(!chunk || chunk->used + size > chunk->capacity) {
        size_t capacity = arena->chunk_size;
        if(size > capacity) capacity = size;

        size_t header = sizeof(ArenaChunk);
        ArenaChunk *new_chunk = malloc(header + capacity);
        if(!new_chunk) return NULL;

        new_chunk->next = chunk;
        new_chunk->capacity = capacity;
        new_chunk->used = 0;
        arena->head = new_chunk;
        chunk = new_chunk;
    }

    void *memory = chunk->data + chunk->used;
    chunk->used += size;
    return memory;
}

void *arena_calloc(Arena *arena, size_t size) {
    void *ptr = arena_alloc(arena, size);
    if(ptr) memset(ptr, 0, size);
    return ptr;
}

char *arena_strdup(Arena *arena, const char *source) {
    if(!source) return NULL;

    size_t len = strlen(source) + 1;
    char *copy = arena_calloc(arena, len);
    if(!copy) return NULL;

    memcpy(copy, source, len);
    return copy;
}

ASTNode *make_node(Arena *arena, enum NodeType type) {
    if(!arena) return NULL;

    ASTNode *node = arena_calloc(arena, sizeof(ASTNode));
    if(!node) return NULL;

    memset(node, 0, sizeof(*node));
    node->type = type;
    node->arena = arena;
    return node;
}

ASTNode *make_number(Arena *arena, double value) {
    ASTNode *node = make_node(arena, NODE_NUMBER);
    if(!node) return NULL;
    node->number.value = value;
    return node;
}

ASTNode *make_false(Arena *arena) {
    ASTNode *node = make_node(arena, NODE_BOOL);
    node->boolean.value = false;
    return node;
}

ASTNode *make_true(Arena *arena) {
    ASTNode *node = make_node(arena, NODE_BOOL);
    node->boolean.value = true;
    return node;
}

ASTNode *make_function_call(Arena *arena, char *name, ASTNode *arg) {
    if(!name || !arena) return NULL;
    ASTNode *node = make_node(arena, NODE_FUNCTION_CALL);
    if(!node) return NULL;
    node->call.name = name;
    node->call.arguments = arg;
    node->call.count = 1;
    return node;
}

ASTNode *make_binary(Arena *arena, enum TokenType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = make_node(arena, NODE_BINARY_OP);
    if(!node) return NULL;
    node->binary.left = left;
    node->binary.op = op;
    node->binary.right = right;
    return node;
}

ASTNode *make_block(Arena *arena) {
    ASTNode *node = make_node(arena, NODE_BLOCK);
    if(!node) return NULL;

    node->block.capacity = 32;
    node->block.children = arena_alloc(arena, node->block.capacity * sizeof(ASTNode *));
    if(!node->block.children) return NULL;
    node->block.count = 0;
    return node;
}

ASTNode *make_variable(Arena *arena, const char *name) {
    if(!arena || !name) return NULL;

    ASTNode *node = make_node(arena, NODE_VARIABLE);
    if(!node) return NULL;
    node->variable.name = arena_strdup(arena, name);
    return node;
}

ASTNode *make_assign(Arena *arena, const char *name, ASTNode *value) {
    if(!arena || !name) return NULL;

    ASTNode *node = make_node(arena, NODE_ASSIGN);
    if(!node) return NULL;
    node->assign.name = arena_strdup(arena, name);
    node->assign.value = value;
    return node;
}

ASTNode *make_unary(Arena *arena, enum TokenType type, ASTNode *right) {
    if(!arena || !right) return NULL;

    ASTNode *node = make_node(arena, NODE_UNARY_OP);
    if(!node) return NULL;
    node->unary.op = type;
    node->unary.right = right;
    return node;
}

ASTNode *make_if(Arena *arena, ASTNode *condition, ASTNode *if_branch,
                 ASTNode *else_branch) {
    if(!arena || !condition || !if_branch) return NULL;
    ASTNode *node = make_node(arena, NODE_IF);
    node->if_statement.condition = condition;
    node->if_statement.if_branch = if_branch;
    node->if_statement.else_branch = else_branch;
    return node;
}

ASTNode *make_while(Arena *arena, ASTNode *condition, ASTNode *body) {
    if(!arena || !condition || !body) return NULL;
    ASTNode *node = make_node(arena, NODE_WHILE);
    node->while_statement.condition = condition;
    node->while_statement.body = body;
    return node;
}

void push_to_block(Arena *arena, ASTNode *parent, ASTNode *child) {
    if(!arena || !parent || !child) return;

    if(parent->block.count >= parent->block.capacity) {
        size_t new_capacity = parent->block.capacity * 2;
        ASTNode **new_children = arena_alloc(arena, new_capacity * sizeof(ASTNode *));
        if(!new_children) return;

        memcpy(new_children, parent->block.children,
               parent->block.count * sizeof(ASTNode *));
        parent->block.children = new_children;
        parent->block.capacity = new_capacity;
    }

    parent->block.children[parent->block.count++] = child;
}
