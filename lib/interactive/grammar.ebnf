/*
 Grammar summary for the embedded scripting language.

 The parser supports expressions, assignments, function calls, and simple control
 flow constructs (`if`, `while`) with indentation-based blocks. Function
 definitions are not supported; only calls to existing builtins are available.
*/

program      = { stmt } EOF ;

stmt         = assignment | if_stmt | while_stmt | expression ;

assignment   = IDENTIFIER "=" expression ;

if_stmt      = "if" expression ":" NEWLINE block [ "else" ":" NEWLINE block ] ;

while_stmt   = "while" expression ":" NEWLINE block ;

block        = INDENT { stmt } DEDENT ;

expression   = equality ;

equality     = comparison { ("==" | "!=") comparison } ;

comparison   = additive { ("<" | ">" | "<=" | ">=") additive } ;

additive     = multiplicative { ("+" | "-") multiplicative } ;

multiplicative = unary { ("*" | "/") unary } ;

unary        = ("-" | "+") unary | primary ;

primary      = NUMBER | TRUE | FALSE | IDENTIFIER ["(" [ arguments ] ")"] | "(" expression ")" ;

arguments    = expression { "," expression } ;

/*
 Notes:
 - IDENTIFIER followed by parentheses denotes a function call (e.g. `print(42)`).
 - Blocks use explicit INDENT/DEDENT tokens produced by the lexer (similar to
   Python-style indentation). Each `if`/`while` requires a trailing colon and a
   newline before the indented block.
 - The parser reports syntax errors with approximate line/column information.
*/
