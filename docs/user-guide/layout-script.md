A script (layout script) is a lightweight, token-bassed notation used to express a sequence of tracks and switch expressions. The tokenizer accepts numbers and nested `SW(...)` expressions; tokens may be separated by white spaces, commas or semicolons.

Syntax summary

- Token: a non-negative integer (e.g. `1`) or a switch expression `SW(...)`.
- Separators: white space, `,` or `;` (all treated equivalently).
- A switch expression must be written as `SW(<content>)` where `<content>` is a comma/space-separated sequence of tokens. The content must begin and end with a number, but may contain nested `SW(...)` expressions.

Comments

- Single-line comments start with `#` or `//` and continue to the end of the line.

Examples

- `1, SW(2, SW(3,4), 5), 6`  — valid: a top-level sequence containing a nested switch.
- `10; 11 SW( 12, 13 ); 14` — equivalent representation using semicolons.

Notes

- The layout parser is tolerant of extra white spaces. Syntax errors (unmatched parentheses, missing numbers) are reported by the tokenizer/parser with a location to help debugging.
