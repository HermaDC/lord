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
        free_lexer_result(&res);
        return 1;
    }
    VM vm = make_VM();
    if(!vm.variables) return 1;
    eval_ast(&vm, root);
    free_ast(root);
    free_lexer_result(&res);
    destroy_VM(&vm);
    return 0;
}

int run_command_line(VM *vm, const char *line) {
    size_t size = strlen(line);
    char *buffer = strdup(line);
    FILE *f = fmemopen(buffer, size, "r");
    LexerResult res = tokenize_file(f);
    ASTNode *root = parse_tokens(res.tokens, res.token_count);
    if(!root) {
        printf("Failed to tokenize file\n");
        free_lexer_result(&res);
        free(buffer);
        fclose(f);
        return 1;
    }
    eval_ast(vm, root);
    free_ast(root);
    free_lexer_result(&res);
    return 0;
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
    VM vm = make_VM();
    if(!vm.variables) return 1;
    while(1) {
        char *line = linenoise(">>> ");
        if(!line) continue;
        linenoiseHistoryAdd(line);
        int err = run_command_line(&vm, line);
        if(err) exit(err);
        if(vm.should_exit) exit(vm.error_code);

        linenoiseFree(line);
    }
    return 0;
}
