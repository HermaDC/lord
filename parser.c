// parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

#define RED "\x1b[1;91m"
#define RESET "\x1b[0m"

#define IS_TOK_DELIMITER(c) (isspace(c) || c == ',' || c == ';')

//TODO: add the creation on sw and tracks sw use a stack
typedef enum {
    NUMBER,
    OPEN,
    CLOSE,
    SW,
} TokenType;

typedef struct {
    int value;
    TokenType type;
    size_t column;
} Token;

typedef enum{
    TOKENIZE_OK,
    TOKENIZE_MISSING_NUM,
    TOKENIZE_UNKNOWN_CHAR,
    TOKENIZE_UNMATCHED_PARENTHESES,
    TOKENIZE_OOM

} TokenizeError;

// 10 SW(10)
//token=10 o token=sw(10)
static int push_token(Token **tokens,
                      size_t *count,
                      size_t *capacity,
                      TokenType type,
                      int value,
                      size_t column)
{
    if (*count >= *capacity) {
        *capacity *= 2;
        Token *tmp = realloc(*tokens, *capacity * sizeof(Token));
        if (!tmp) return 0;
        *tokens = tmp;
    }

    (*tokens)[*count].type = type;
    (*tokens)[*count].value = value;
    (*tokens)[*count].column = column;

    (*count)++;
    return 1;
}

void print_error_at(const char *input, size_t column) {
    fprintf(stderr, "%s\n", input);

    for (size_t i = 0; i < column; i++)
        fputc(' ', stderr);

    fprintf(stderr, RED "^\n" RESET);
}

Token *tokenize(const char *str, size_t *num_tokens, TokenizeError *error_code) {
    size_t buffer_size = 1;
    *num_tokens = 0;

    char *token_str = strdup(str);

    if(!token_str) goto error;
    Token *tokens = malloc(buffer_size * sizeof(Token));
    if(!tokens) goto error;

    size_t i = 0;
    while(isspace(token_str[i])) i++;
    while(token_str[i] != '\0'){
        char c = token_str[i];
        if(IS_TOK_DELIMITER(c)){ //use ,; as delimiters
            i++;
            continue;
        }
        if(isdigit(c)){
            int value = 0;
            size_t start = 0;
            while(isdigit(token_str[i])){
                value = value * 10 + (token_str[i] - '0');
                i++; 
            }
            if(!push_token(&tokens, num_tokens, &buffer_size, NUMBER, value, start)) goto error;
        } else if(c == '('){
            if(!push_token(&tokens, num_tokens, &buffer_size, OPEN, 0, i)) goto error;
            i++;
        } else if(c == ')'){
            if(!push_token(&tokens, num_tokens, &buffer_size, CLOSE, 0, i)) goto error;
            i++;
        } 
        // If is SW add a token with type SW and value 0
        else if(strncmp(&token_str[i], "SW", 2) == 0){
            if(!push_token(&tokens, num_tokens, &buffer_size, SW, 0, i)) goto error;
            i += 2;
        }
        else if(c == '#' || strncmp(&token_str[i], "//", 2) == 0){
            if(tokens) return tokens; //The rest of the string should be treated like a comment
        }
        else {
            fprintf(stderr, RED "Syntax Error: " RESET "Unexpected character '%c'\n"
                "Column: %zu\n", c, i);
            if(c=='s' || c == 'w') fprintf(stderr, "Perhaps you meant SW\n");
            print_error_at(str, i);
            *error_code = TOKENIZE_UNKNOWN_CHAR;

            if(token_str) free(token_str);
            if(tokens) free(tokens);
            return NULL;  
            
        }
    }

    return tokens;

error:
    if(token_str) free(token_str);
    if(tokens) free(tokens);
    *error_code = TOKENIZE_OOM;
    return NULL;    
}

void print_tokens(Token *tokens_arr, size_t count){
    for(size_t i = 0; i < count; i++){
        if(tokens_arr[i].type == NUMBER){
            printf("NUM: %d\n", tokens_arr[i].value);
        } else if(tokens_arr[i].type == OPEN){
            printf("OPEN\n");
        } else if(tokens_arr[i].type == CLOSE){
            printf("CLOSE\n");
        }
        else if(tokens_arr[i].type == SW){
            printf("SW\n");
        }
    }
}

int check_syntax(Token *tokens_arr, char *original_str, size_t count, TokenizeError *exit_code){ //TODO add the exit_code or error code
    int balance = 0;
    if(tokens_arr[0].type != NUMBER){
        fprintf(stderr, RED "Syntax Error:" RESET " The config file must begin with a number\n"
            "Column: %zu\n", tokens_arr[0].column );
        *exit_code = TOKENIZE_MISSING_NUM;
        print_error_at(original_str, tokens_arr[0].column);
        return -1;
    }
    if(tokens_arr[count-1].type != NUMBER){
        fprintf(stderr, RED "Syntax Error:" RESET " The config file must end with a number\n"
            "Column: %zu\n", tokens_arr[count-1].column );
        *exit_code = TOKENIZE_MISSING_NUM;
        print_error_at(original_str, tokens_arr[count-1].column);
        return -1;
    }
    for(size_t i = 0; i < count; i++){
        if (tokens_arr[i].type == SW){
            if(i + 1 >= count || tokens_arr[i + 1].type != OPEN){
                fprintf(stderr, RED "Syntax Error:" RESET " SW must be followed by an opening parenthesis\n"
                    "Column: %zu\n", tokens_arr[i].column );
                *exit_code = TOKENIZE_UNMATCHED_PARENTHESES;
                print_error_at(original_str, tokens_arr[i].column);
                return -1;
            }
        }
        else if(tokens_arr[i].type == OPEN){
            balance++;
        } else if(tokens_arr[i].type == CLOSE){
            balance--;
            if(balance < 0){
                fprintf(stderr, RED "Syntax Error: " RESET "Unmatched closing parenthesis\n"
                    "Column: %zu\n", tokens_arr[i].column);
                *exit_code = TOKENIZE_UNMATCHED_PARENTHESES;
                print_error_at(original_str, tokens_arr[i].column);
                return -1;
            } //the previous of the close parenthesis should be a number
            if(tokens_arr[i-1].type != NUMBER){
                fprintf(stderr, RED "Syntax Error: " RESET "Missing number before closing parenthesis\n"
                    "Column: %zu\n", tokens_arr[i].column);
                *exit_code = TOKENIZE_MISSING_NUM;
                print_error_at(original_str, tokens_arr[i].column-1);
                return -1;
            }
        }
    }
    if(balance > 0){
        fprintf(stderr, RED "Syntax Error: " RESET "Unmatched opening parenthesis\n");
        *exit_code = TOKENIZE_UNMATCHED_PARENTHESES;
        print_error_at(original_str, original_str ? strlen(original_str) : 0);
        return -1;
    }
    *exit_code = TOKENIZE_OK;
    printf("Syntax is correct\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_to_tokenize>\n", argv[0]);
        return EXIT_FAILURE;
    }
    size_t count;
    Token *tokens = NULL;
    printf("%s\n",argv[1]);
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        fclose(file);
        return EXIT_FAILURE;
    }
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file) != -1) {
        line[strcspn(line, "\n")] = 0;
        printf("Processing line: %s\n", line);
        TokenizeError error;
        tokens = tokenize(line, &count, &error);
        if(tokens){ 
            print_tokens(tokens, count);
            check_syntax(tokens, line, count, &error);
        }
        if(tokens) free(tokens);
    }
    free(line);
    fclose(file);
    /*if(argc > 1){
        tokens = tokenize(argv[1], &count);
    }
    else{
        fprintf(stderr, "Usage: %s <file_to_tokenize>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if(tokens){ 
        print_tokens(tokens, count);
        check_syntax(tokens, argv[1], count);
    }

    if(tokens) free(tokens);
    return 0;*/
}
