// parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "config.h"

#define IS_TOK_DELIMITER(c) (isspace(c) || c == ',' || c == ';')

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

    if(!str || strlen(str)<1){
        *error_code = TOKENIZE_EMPTY_STR;
        return NULL;
    }

    Token *tokens = NULL;
    tokens = malloc(buffer_size * sizeof(Token));
    if(!tokens) goto error;

    size_t i = 0;
    while(isspace(str[i])) i++;
    while(str[i] != '\0'){
        char c = str[i];
        if(IS_TOK_DELIMITER(c)){ //use ,; as delimiters
            i++;
            continue;
        }
        if(isdigit(c)){
            int value = 0;
            size_t start = i;
            while(isdigit(str[i])){
                value = value * 10 + (str[i] - '0');
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
        else if(strncmp(&str[i], "SW", 2) == 0){
            if(!push_token(&tokens, num_tokens, &buffer_size, SW, 0, i)) goto error;
            i += 2;
        }
        else if(c == '#' || strncmp(&str[i], "//", 2) == 0){
            *error_code = TOKENIZE_OK;
            // Stop processing at comment, return current tokens
            if (*num_tokens == 0) {
                free(tokens);
                return NULL;
            }
            return tokens;
        }
        else {
            fprintf(stderr, RED "Syntax Error: " RESET "Unexpected character '%c'\n"
                "Column: %zu\n", c, i);
            if(c=='s' || c == 'w') fprintf(stderr, "Perhaps you meant SW\n");
            print_error_at(str, i);
            *error_code = TOKENIZE_UNKNOWN_CHAR;

            if(tokens) free(tokens);
            return NULL;  
            
        }
    }

    return tokens;

error:
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

//TODO use the erorcode
int check_syntax(Token *tokens_arr, char *original_str, size_t count, TokenizeError *error_code){
    if(count == 0){
        *error_code = TOKENIZE_OK;
        return 0;
    }
    int balance = 0;
    if(tokens_arr[0].type != NUMBER){
        fprintf(stderr, RED "Syntax Error:" RESET " The config file must begin with a number\n"
            "Column: %zu\n", tokens_arr[0].column );
        *error_code = TOKENIZE_MISSING_NUM;
        print_error_at(original_str, tokens_arr[0].column);
        return -1;
    }
    if(tokens_arr[count-1].type != NUMBER){
        fprintf(stderr, RED "Syntax Error:" RESET " The config file must end with a number\n"
            "Column: %zu\n", tokens_arr[count-1].column );
        *error_code = TOKENIZE_MISSING_NUM;
        print_error_at(original_str, tokens_arr[count-1].column);
        return -1;
    }
    for(size_t i = 0; i < count; i++){
        if (tokens_arr[i].type == SW){
            if(i + 1 >= count || tokens_arr[i + 1].type != OPEN){
                fprintf(stderr, RED "Syntax Error:" RESET " SW must be followed by an opening parenthesis\n"
                    "Column: %zu\n", tokens_arr[i].column );
                *error_code = TOKENIZE_UNMATCHED_PARENTHESES;
                print_error_at(original_str, tokens_arr[i].column);
                return -1;
            }
        }
        else if(tokens_arr[i].type == NUMBER) {
            if(tokens_arr[i].value <= 0) {
                fprintf(stderr, RED "Syntax Error:" RESET " Number of tracks must be greater than 0\n"
                    "Column: %zu\n", tokens_arr[i].column );
                *error_code = TOKENIZE_MISSING_NUM;
                print_error_at(original_str, tokens_arr[i].column);
                return -1;
            }
        }
        else if(tokens_arr[i].type == OPEN){
            balance++;
            if(i + 1 >= count || tokens_arr[i+1].type != NUMBER){
                fprintf(stderr, RED "Syntax Error:" RESET " Parenthesis must be followed by a valid number\n"
                    "Column: %zu\n", tokens_arr[i].column );
                *error_code = TOKENIZE_MISSING_NUM;
                print_error_at(original_str, tokens_arr[i].column);
                return -1;
            }
            if(tokens_arr[i+1].value <=0){
                fprintf(stderr, RED "Syntax Error:" RESET " Number of tracks must be greater than 0\n"
                    "Column: %zu\n", tokens_arr[i+1].column );
                *error_code = TOKENIZE_MISSING_NUM;
                print_error_at(original_str, tokens_arr[i+1].column);
                return -1;
            }
        } else if(tokens_arr[i].type == CLOSE){
            balance--;
            if(balance < 0){
                fprintf(stderr, RED "Syntax Error: " RESET "Unmatched closing parenthesis\n"
                    "Column: %zu\n", tokens_arr[i].column);
                *error_code = TOKENIZE_UNMATCHED_PARENTHESES;
                print_error_at(original_str, tokens_arr[i].column);
                return -1;
            } //the previous of the close parenthesis should be a number
            if(i==0 || tokens_arr[i-1].type != NUMBER){
                fprintf(stderr, RED "Syntax Error: " RESET "Missing number before closing parenthesis\n"
                    "Column: %zu\n", tokens_arr[i].column);
                *error_code = TOKENIZE_MISSING_NUM;
                print_error_at(original_str, tokens_arr[i].column-1);
                return -1;
            }
        }
        if(balance > MAX_STACK_SIZE){
            fprintf(stderr, RED "Syntax Error: " RESET "To many switches stacked.\n"
                    "Column: %zu\n", tokens_arr[i].column);
                *error_code = TOKENIZE_SWITCH_STACK_OVERFLOW;
                print_error_at(original_str, tokens_arr[i].column-1);
                return -1;

        }
    }
    if(balance > 0){
        fprintf(stderr, RED "Syntax Error: " RESET "Unmatched opening parenthesis\n");
        *error_code = TOKENIZE_UNMATCHED_PARENTHESES;
        print_error_at(original_str, original_str ? strlen(original_str) : 0);
        return -1;
    }
    *error_code = TOKENIZE_OK;
    return 0;
}

