#include "parser.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser-utils.h"

typedef struct {
    Token *tokens;
    size_t pos;
    size_t count;
    Arena *arena;

    bool has_error;
    enum ParserErrorCode error;
} Parser;

static inline Token *peek(Parser *p) { return &p->tokens[p->pos]; }

static inline Token *peek_next(Parser *p) {
    if(p->pos + 1 >= p->count) return peek(p);

    return &p->tokens[p->pos + 1];
}

static inline Token *previous(Parser *p) {
    if(p->pos != 0) return &p->tokens[p->pos - 1];
    return &p->tokens[0];
}

static int is_at_end(Parser *p) { return peek(p)->type == TOKEN_EOF; }

static Token *advance(Parser *p) {
    if(!is_at_end(p)) return &p->tokens[p->pos++];
    return previous(p);
}

static int check(Parser *p, enum TokenType type) {
    if(is_at_end(p)) return 0;
    return peek(p)->type == type;
}

static inline int match(Parser *p, enum TokenType type) {
    if(check(p, type)) {
        advance(p);
        return 1;
    } else
        return 0;
}
static int skip_newlines(Parser *p) {

    int count = 0;
    while(match(p, TOKEN_NEWLINE))
        count++;
    return count;
}
static void print_error_at(const char *error_str, const char *line_str,
                           Token actual_token) {
    printf("An error occur at line %d\n", actual_token.line);
    printf("SyntaxError: %s\n", error_str);
    if(!line_str) return;

    printf("%s", line_str);
    for(int i = 0; i < actual_token.column; i++)
        putc(' ', stdout);
    putc('^', stdout);
}
void parser_error(Parser *p, const char *error_str) {
    p->has_error = true;
    p->error = SYNTAX_ERROR;
    print_error_at(error_str, NULL, *peek(p));
}

void free_ast(ASTNode *node) {
    if(!node || !node->arena) return;
    arena_destroy(node->arena);
}

// Forward declaration
//
ASTNode *parse_primary(Parser *p);
ASTNode *parse_unary(Parser *p);
ASTNode *parse_factor(Parser *p);
ASTNode *parse_term(Parser *p);
ASTNode *parse_comparison(Parser *p);
ASTNode *parse_equality(Parser *p);
ASTNode *parse_expression(Parser *p);
ASTNode *parse_statatement(Parser *p);

ASTNode *parse_primary(Parser *p) {
    if(p->has_error) return NULL;

    if(match(p, TOKEN_NUMBER)) {
        Token *tok = previous(p);
        if(tok->lexeme) return make_number(p->arena, atof(tok->lexeme));
    }
    if(match(p, TOKEN_TRUE_LITERAL)) { return make_true(p->arena); }
    if(match(p, TOKEN_FALSE_LITERAL)) { return make_false(p->arena); }
    if(match(p, TOKEN_IDENTIFIER)) {
        Token *tok = previous(p);
        if(match(p, TOKEN_LEFT_PAREN)) {
            ASTNode *arg = NULL;
            if(!check(p, TOKEN_RIGHT_PAREN)) { arg = parse_expression(p); }
            if(!match(p, TOKEN_RIGHT_PAREN)) {
                parser_error(p, "expected ')'");
                return NULL;
            }
            return make_function_call(p->arena, tok->lexeme, arg);
        }
        return make_variable(p->arena, tok->lexeme);
    }
    if(match(p, TOKEN_LEFT_PAREN)) {
        ASTNode *left = parse_expression(p);
        if(!match(p, TOKEN_RIGHT_PAREN)) { parser_error(p, "SyntaxError: expected ')'"); }
        return left;
    }
    printf("parse_primary token=%s\n", token_type_to_str(peek(p)->type));
    parser_error(p, "SyntaxError: expected expression");
    return NULL;
}

ASTNode *parse_unary(Parser *p) {
    if(p->has_error) return NULL;

    if(match(p, TOKEN_MINUS) || match(p, TOKEN_PLUS)) {
        Token *op = previous(p);
        ASTNode *right = parse_unary(p);
        if(!right) return NULL;
        return make_unary(p->arena, op->type, right);
    }
    return parse_primary(p);
}

ASTNode *parse_factor(Parser *p) {
    if(p->has_error) return NULL;

    ASTNode *node = parse_unary(p);
    while(match(p, TOKEN_STAR) || match(p, TOKEN_SLASH)) {
        Token *op = previous(p);
        ASTNode *right = parse_unary(p);
        if(!right) return NULL;
        node = make_binary(p->arena, op->type, node, right);
    }
    return node;
}

ASTNode *parse_term(Parser *p) {
    if(p->has_error) return NULL;

    ASTNode *node = parse_factor(p);
    while(match(p, TOKEN_PLUS) || match(p, TOKEN_MINUS)) {
        Token *op = previous(p);
        ASTNode *right = parse_factor(p);
        if(!right) return NULL;
        node = make_binary(p->arena, op->type, node, right);
    }
    return node;
}

ASTNode *parse_comparison(Parser *p) {
    if(p->has_error) return NULL;

    ASTNode *node = parse_term(p);
    while(match(p, TOKEN_LESS) || match(p, TOKEN_LESS_EQUAL) || match(p, TOKEN_GREATER) ||
          match(p, TOKEN_GREATER_EQUAL)) {
        Token *op = previous(p);
        ASTNode *right = parse_term(p);
        if(!right) return NULL;
        node = make_binary(p->arena, op->type, node, right);
    }
    return node;
}

ASTNode *parse_equality(Parser *p) {
    if(p->has_error) return NULL;

    ASTNode *node = parse_comparison(p);
    while(match(p, TOKEN_EQUAL) || match(p, TOKEN_NOT_EQUAL)) {
        Token *op = previous(p);
        ASTNode *right = parse_comparison(p);
        if(!right) return NULL;
        node = make_binary(p->arena, op->type, node, right);
    }
    return node;
}

ASTNode *parse_expression(Parser *p) {
    if(p->has_error) return NULL;
    return parse_equality(p);
}

ASTNode *parse_assignment(Parser *p) {
    if(p->has_error) return NULL;

    Token *name = advance(p);
    if(!match(p, TOKEN_ASSIGN)) { // consume '='
        parser_error(p, "Expected '='");
        return NULL;
    }

    ASTNode *value = parse_expression(p);

    return make_assign(p->arena, name->lexeme, value);
}

static ASTNode *parse_block(Parser *p) {
    if(p->has_error) return NULL;

    if(!match(p, TOKEN_NEWLINE)) {
        parser_error(p, "Expected newline");
        return NULL;
    }

    skip_newlines(p);

    if(!match(p, TOKEN_INDENT)) {
        parser_error(p, "Expected indentation");
        return NULL;
    }

    ASTNode *block = make_block(p->arena);
    printf("Parsing block, indent condume\n");
    while(!check(p, TOKEN_DEDENT) && !is_at_end(p)) {
        skip_newlines(p);
        if(check(p, TOKEN_DEDENT)) { break; }
        ASTNode *stmt = parse_statatement(p);

        if(!stmt) return NULL;

        push_to_block(p->arena, block, stmt);
        printf("Statatement parse\n");
    }

    if(!match(p, TOKEN_DEDENT)) {
        parser_error(p, "Expected dedent");
        return NULL;
    }

    return block;
}

ASTNode *parse_if(Parser *p) {
    if(p->has_error) return NULL;

    ASTNode *condition = parse_expression(p);

    if(!condition) return NULL;

    if(!match(p, TOKEN_COLON)) {
        parser_error(p, "Expected ':' after if condition");
        return NULL;
    }

    ASTNode *if_body = parse_block(p);

    if(!if_body) return NULL;

    skip_newlines(p);

    ASTNode *else_body = NULL;

    if(match(p, TOKEN_ELSE)) {
        if(!match(p, TOKEN_COLON)) {
            parser_error(p, "Expected ':' after else");
            return NULL;
        }
        else_body = parse_block(p);

        if(!else_body) return NULL;
    }

    return make_if(p->arena, condition, if_body, else_body);
}
ASTNode *parse_while(Parser *p) {
    if(p->has_error) return NULL;

    ASTNode *condition = parse_expression(p);

    if(!condition) return NULL;

    if(!match(p, TOKEN_COLON)) {
        parser_error(p, "Expected ':' after while condition");
        return NULL;
    }

    ASTNode *body = parse_block(p);
    if(!body) return NULL;
    return make_while(p->arena, condition, body);
}
static void print_spaces(int n) {
    for(int i = 0; i < n; i++)
        putchar(' ');
}

static void print_ast_rec(ASTNode *root, int indent) {
    if(!root) {
        print_spaces(indent);
        printf("<nil>\n");
        return;
    }

    switch(root->type) {
    case NODE_BLOCK:
        print_spaces(indent);
        printf("Block\n");
        for(size_t i = 0; i < root->block.count; i++) {
            print_ast_rec(root->block.children[i], indent + 2);
        }
        break;

    case NODE_IF:
        print_spaces(indent);
        printf("If\n");
        print_spaces(indent + 2);
        printf("Condition:\n");
        print_ast_rec(root->if_statement.condition, indent + 4);
        print_spaces(indent + 2);
        printf("Then:\n");
        print_ast_rec(root->if_statement.if_branch, indent + 4);
        if(root->if_statement.else_branch) {
            print_spaces(indent + 2);
            printf("Else:\n");
            print_ast_rec(root->if_statement.else_branch, indent + 4);
        }
        break;

    case NODE_WHILE:
        print_spaces(indent);
        printf("While\n");
        print_spaces(indent + 2);
        printf("Condition:\n");
        print_ast_rec(root->while_statement.condition, indent + 4);
        print_spaces(indent + 2);
        printf("Body:\n");
        print_ast_rec(root->while_statement.body, indent + 4);
        break;
    case NODE_FUNCTION_CALL:
        print_spaces(indent);
        printf("%s", root->call.name);
        printf("(");
        if(root->call.arguments) {
            print_spaces(indent);
            printf("\n");
            print_ast_rec(root->call.arguments, indent + 4);
            print_spaces(indent);
        }
        printf(")\n");
        break;

    case NODE_ASSIGN:
        print_spaces(indent);
        printf("Assign: %s\n", root->assign.name ? root->assign.name : "(null)");
        print_ast_rec(root->assign.value, indent + 2);
        break;

    case NODE_VARIABLE:
        print_spaces(indent);
        printf("Var: %s\n", root->variable.name ? root->variable.name : "(null)");
        break;

    case NODE_NUMBER:
        print_spaces(indent);
        printf("Number: %f\n", root->number.value);
        break;
    case NODE_BOOL:
        print_spaces(indent);
        printf("Bool: %s\n", root->boolean.value ? "True" : "False");
        break;

    case NODE_BINARY_OP:
        print_spaces(indent);
        printf("BinaryOp: %s\n", token_type_to_str(root->binary.op));
        print_ast_rec(root->binary.left, indent + 2);
        print_ast_rec(root->binary.right, indent + 2);
        break;

    case NODE_UNARY_OP:
        print_spaces(indent);
        printf("UnaryOp: %s\n", token_type_to_str(root->unary.op));
        print_ast_rec(root->unary.right, indent + 2);
        break;

    default:
        print_spaces(indent);
        printf("<unknown node type %d>\n", root->type);
        break;
    }
}

void print_ast(ASTNode *root) { print_ast_rec(root, 0); }

ASTNode *parse_statatement(Parser *p) {
    if(p->has_error) return NULL;

    ASTNode *node = NULL;

    // assignment
    if(check(p, TOKEN_IDENTIFIER) && peek_next(p)->type == TOKEN_ASSIGN) {

        node = parse_assignment(p);

    } else if(match(p, TOKEN_IF)) {
        node = parse_if(p);

    } else if(match(p, TOKEN_WHILE)) {
        node = parse_while(p);
    } else {

        node = parse_expression(p);
    }

    return node;
}

ASTNode *parse_tokens(Token *tokens, size_t count) {
    // Init Parser
    Parser par;
    par.tokens = tokens;
    par.count = count;
    par.pos = 0;
    par.has_error = false;
    par.error = PARSE_OK;
    par.arena = arena_create(0);
    if(!par.arena) return NULL;

    ASTNode *root = make_block(par.arena);
    if(!root) {
        arena_destroy(par.arena);
        return NULL;
    }

    while(!is_at_end(&par)) {
        if(par.has_error) break;

        skip_newlines(&par);
        if(is_at_end(&par)) break;
        ASTNode *node = parse_statatement(&par);
        if(!node) {
            if(is_at_end(&par)) break;
            parser_error(&par, "parser error");
            break;
        }
        push_to_block(par.arena, root, node);
    }

    if(par.has_error) {
        free_ast(root);
        return NULL;
    }
#ifdef DEBUG
    print_ast(root);
#endif

    return root;
}
