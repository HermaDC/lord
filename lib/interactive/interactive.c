#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../linenoise-lib/linenoise.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "vm/builtins.h"
#include "vm/eval.h"

int run_script_file(FILE *f) {
    LexerResult res = tokenize_file(f);
    ASTNode *root = parse_tokens(res.tokens, res.token_count);
    if(!root) {
        printf("Failed to tokenize file\n");
        return 1;
    }
    eval_ast(root);
    return 0;
}

int run_command_line(const char *line) {
    size_t size = strlen(line);
    char *buffer = strdup(line);
    FILE *f = fmemopen(buffer, size, "r");
    LexerResult res = tokenize_file(f);
    ASTNode *root = parse_tokens(res.tokens, res.token_count);
    if(!root) {
        printf("Failed to tokenize file\n");
        free(buffer);
        return 1;
    }

    eval_ast(root);
    return 0;
    free(buffer);
}
void completion(const char *buf, linenoiseCompletions *lc) {
    for(size_t i = 0; i < count_call_options; i++) {
        if(strncmp(buf, funcs[i].name, strlen(buf)) == 0) {
            linenoiseAddCompletion(lc, funcs[i].name);
        }
    }
}

int run_interactive_loop(void) {
    linenoiseSetCompletionCallback(completion);
    while(1) {
        char *line = linenoise(">>> ");
        if(!line) continue;
        linenoiseHistoryAdd(line);
        run_command_line(line);

        linenoiseFree(line);
    }
    return 0;
}
