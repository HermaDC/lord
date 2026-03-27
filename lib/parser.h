#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h"

//Prints the array of tokens
void print_tokens(Token *tokens_arr, size_t count);

//Given a null-terminated string, returns an array of num_tokens length, and if an error happens, it stores it in error_code
Token *tokenize(const char *str, size_t *num_tokens, TokenizeError *error_code);

//Checks if the syntax is correct, if not, it stores the specific error on error_code
int check_syntax(Token *tokens_arr, char *original_str, size_t count, TokenizeError *error_code);
