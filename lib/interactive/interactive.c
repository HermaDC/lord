#include <stdio.h>

#include "parser/lexer.h"
#include "parser/parser.h"
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
