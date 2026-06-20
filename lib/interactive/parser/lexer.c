#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define is_digit(c) (((c) >= '0' && (c) <= '9'))

typedef struct {
    Token *token_arr;
    size_t buffer;
    size_t count;
} Tokenizator;

static inline int init_tokenizator(Tokenizator *tk) {
    tk->buffer = 32;
    tk->count = 0;
    tk->token_arr = malloc(tk->buffer * sizeof(Token));
    if(!tk->token_arr) {
        tk->buffer = 0;
        tk->count = 0;
        return 1;
    }
    return 0;
}
static bool is_blank_line(const char *line) {
    while(*line == ' ' || *line == '\t')
        line++;

    return *line == '\n' || *line == '\0';
}
void free_tokenizator(Tokenizator *tk) {
    for(size_t i = 0; i < tk->count; i++) {
        enum TokenType actual_type = tk->token_arr[i].type;
        switch(actual_type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_NUMBER:
            free(tk->token_arr[i].lexeme);
            break;
        default:
            break;
        }
    }
    tk->buffer = 0;
    tk->count = 0;
    free(tk->token_arr);
}

void free_lexer_result(LexerResult *result) {
    if(!result) return;

    if(result->tokens) {
        for(size_t i = 0; i < result->token_count; i++) {
            enum TokenType actual_type = result->tokens[i].type;
            switch(actual_type) {
            case TOKEN_IDENTIFIER:
            case TOKEN_NUMBER:
                free(result->tokens[i].lexeme);
                break;
            default:
                break;
            }
        }
        free(result->tokens);
        result->tokens = NULL;
        result->token_count = 0;
    }

    if(result->lines) {
        for(size_t i = 0; i < result->line_count; i++) {
            free(result->lines[i]);
        }
        free(result->lines);
        result->lines = NULL;
        result->line_count = 0;
    }
}

static inline int push_token(Tokenizator *tk, const Token tok) {
    if(tk->count >= tk->buffer) {
        tk->buffer *= 2;
        Token *temp_tokens = realloc(tk->token_arr, tk->buffer * sizeof(Token));
        if(!temp_tokens) { return 1; }
        tk->token_arr = temp_tokens;
    }
    tk->token_arr[tk->count++] = tok;
    return 0;
}

void print_tokens(const Token *t, size_t count) {
    for(size_t i = 0; i < count; i++) {
        if(t[i].lexeme)
            printf("Token: %s; Value %s\n", token_type_to_str(t[i].type), t[i].lexeme);
        else
            printf("Token: %s\n", token_type_to_str(t[i].type));
    }
}

size_t indent_stack[MAX_INDENTS];
size_t indent_top = 0;

static inline int get_indent(const char *line) {
    int i = 0;
    while(line[i] == ' ')
        i++;
    return i;
}

void tokenize_line(const char *line, const int line_num, Tokenizator *tk) {
    int i = 0;
    bool in_str = false;

    if(is_blank_line(line)) {
        push_token(tk, (Token){TOKEN_NEWLINE, NULL, 0, line_num});
        return;
    }

    // check for identattion
    size_t indent = get_indent(line);
    if(indent > indent_stack[indent_top]) {
        indent_stack[++indent_top] = indent;

        push_token(tk, (Token){TOKEN_INDENT, NULL, 0, line_num});
    } else {
        while(indent < indent_stack[indent_top]) {
            indent_top--;

            push_token(tk, (Token){TOKEN_DEDENT, NULL, 0, line_num});
        }

        if(indent != indent_stack[indent_top]) {
            fprintf(stderr, "Indentation error\n");
            return;
        }
    }
    while(line[i]) {
        if(in_str && line[i] != '"') {
            i++;
            continue;
        }
        if(is_digit(line[i])) {
            int start = i;

            while(is_digit(line[i]) || line[i] == '.')
                i++;

            int len = i - start;
            char *lexeme = malloc(len + 1);
            strncpy(lexeme, &line[start], len);
            lexeme[len] = 0;
            push_token(tk, (Token){TOKEN_NUMBER, lexeme, start, line_num});

            // i--
            continue;
        }
        if(isalpha(line[i])) {
            size_t start = i;
            while(isalpha(line[i]) || is_digit(line[i]) || line[i] == '_')
                i++;
            size_t len = i - start;
            // TODO: transfor this in a table search
            char *lexeme = malloc(len + 1);
            strncpy(lexeme, &line[start], len);
            lexeme[len] = 0;
            if(strcmp(lexeme, "False") == 0) {
                push_token(tk, (Token){TOKEN_FALSE_LITERAL, NULL, start, line_num});
                free(lexeme);
                continue;
            } else if(strcmp(lexeme, "True") == 0) {
                push_token(tk, (Token){TOKEN_TRUE_LITERAL, NULL, start, line_num});
                free(lexeme);
                continue;
            } else if(strcmp(lexeme, "if") == 0) {
                push_token(tk, (Token){TOKEN_IF, NULL, start, line_num});
                continue;
            } else if(strcmp(lexeme, "else") == 0) {
                push_token(tk, (Token){TOKEN_ELSE, NULL, start, line_num});
                continue;
            } else if(strcmp(lexeme, "while") == 0) {
                push_token(tk, (Token){TOKEN_WHILE, NULL, start, line_num});
                continue;
            }

            push_token(tk, (Token){TOKEN_IDENTIFIER, lexeme, start, line_num});
            continue;
        }

        switch(line[i]) {
        case '\n':
            push_token(tk, (Token){TOKEN_NEWLINE, NULL, i, line_num});
            break;
        case '"':
            in_str = !in_str;
            break;
        case '+':
            push_token(tk, (Token){TOKEN_PLUS, NULL, i, line_num});
            break;
        case '-':
            push_token(tk, (Token){TOKEN_MINUS, NULL, i, line_num});
            break;
        case '*':
            push_token(tk, (Token){TOKEN_STAR, NULL, i, line_num});
            break;
        case '/':
            push_token(tk, (Token){TOKEN_SLASH, NULL, i, line_num});
            break;
        case '=':
            if(line[i + 1] == '=') {
                push_token(tk, (Token){TOKEN_EQUAL, NULL, i, line_num});
                i++;
                break;
            }
            push_token(tk, (Token){TOKEN_ASSIGN, NULL, i, line_num});
            break;
        case '(':
            push_token(tk, (Token){TOKEN_LEFT_PAREN, NULL, i, line_num});
            break;
        case ')':
            push_token(tk, (Token){TOKEN_RIGHT_PAREN, NULL, i, line_num});
            break;

        case '<':
            if(line[i + 1] == '=') {
                push_token(tk, (Token){TOKEN_LESS_EQUAL, NULL, i, line_num});
                i++;
                break;
            }
            push_token(tk, (Token){TOKEN_LESS, NULL, i, line_num});
            break;
        case '>':
            if(line[i + 1] == '=') {
                push_token(tk, (Token){TOKEN_GREATER_EQUAL, NULL, i, line_num});
                i++;
                break;
            }
            push_token(tk, (Token){TOKEN_GREATER, NULL, i, line_num});
            break;

        case '!':
            if(line[i + 1] == '=') {
                push_token(tk, (Token){TOKEN_NOT_EQUAL, NULL, i, line_num});
                i++;
                break;
            } else {
                printf("Unknow use of token '!'");
                break;
            }
        case ':':
            push_token(tk, (Token){TOKEN_COLON, NULL, i, line_num});
            break;
        case '#':
            return;
        }
        i++;
    }
}

LexerResult tokenize_file(FILE *f) {
    LexerResult result = {0};

    if(!f) return result;

    Tokenizator tk;
    if(init_tokenizator(&tk)) return result;

    size_t lines_capacity = 128;

    result.lines = malloc(lines_capacity * sizeof(char *));
    if(!result.lines) {
        free_tokenizator(&tk);
        return result;
    }

    char *line = NULL;
    size_t n = 0;
    int line_num = 0;

    while(getline(&line, &n, f) != -1) {

        tokenize_line(line, line_num, &tk);

        if(result.line_count >= lines_capacity) {
            lines_capacity *= 2;

            char **tmp = realloc(result.lines, lines_capacity * sizeof(char *));

            if(!tmp) {
                free(line);
                free_tokenizator(&tk);
                return result;
            }

            result.lines = tmp;
        }

        result.lines[result.line_count++] = strdup(line);

        line_num++;
    }

    while(indent_top > 0) {
        indent_top--;

        push_token(&tk, (Token){TOKEN_DEDENT, NULL, 0, line_num});
    }

    push_token(&tk, (Token){TOKEN_EOF, NULL, 0, line_num});

    free(line);

    result.tokens = tk.token_arr;
    result.token_count = tk.count;

#ifdef DEBUG
    print_tokens(tk.token_arr, tk.count);
#endif

    return result;
}
