#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_INDENTS 128

enum TokenType {
    TOKEN_EOF = -1,
    TOKEN_NEWLINE = 0,
    TOKEN_IDENTIFIER,

    // literals
    TOKEN_NUMBER,
    TOKEN_STR_LITERAL,
    TOKEN_FALSE_LITERAL,
    TOKEN_TRUE_LITERAL,

    // Branching
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,

    // separators
    TOKEN_COMA,
    TOKEN_COLON,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_INDENT,
    TOKEN_DEDENT,

    // operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_ASSIGN,

    // Comparison
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL
};

typedef struct {
    enum TokenType type;
    char *lexeme;
    int column;
    int line;
} Token;

typedef struct {
    Token *tokens;
    size_t token_count;

    char **lines;
    size_t line_count;
} LexerResult;

static inline const char *token_type_to_str(enum TokenType type) {
    switch (type) {
    case TOKEN_NUMBER: return "NUMBER";
    case TOKEN_STR_LITERAL: return "STR_LITERAL";
    case TOKEN_IDENTIFIER: return "IDENTIFIER";
    case TOKEN_ASSIGN: return "ASSIGN";
    case TOKEN_COMA: return "COMA";
    case TOKEN_PLUS: return "PLUS";
    case TOKEN_MINUS: return "MINUS";
    case TOKEN_STAR: return "STAR";
    case TOKEN_SLASH: return "BAR";
    case TOKEN_EOF: return "EOF";
    case TOKEN_NEWLINE: return "NEWLINE";
    case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
    case TOKEN_EQUAL: return "EQUAL";
    case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
    case TOKEN_LESS: return "LESS";
    case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
    case TOKEN_GREATER: return "GREATER";
    case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
    case TOKEN_INDENT: return "INDENT";
    case TOKEN_DEDENT: return "DEDENT";
    case TOKEN_IF: return "IF";
    case TOKEN_ELSE: return "ELSE";
    case TOKEN_WHILE: return "WHILE";
    case TOKEN_FALSE_LITERAL: return "FALSE";
    case TOKEN_TRUE_LITERAL: return "TRUE";
    case TOKEN_COLON: return "COLON";
    }

    return "UNKONW";
}

LexerResult tokenize_file(FILE *f);
