# Interactive mode

Start the REPL with `lord -i` or `lord --interactive`. The interactive shell provides a lightweight command interface to inspect, update and persist loaded systems and to run ad‑hoc script expressions using the embedded VM.

The REPL features:

- Prompt: `>>>` with line editing and history (powered by `linenoise`).
- Tab completion for builtin function names.

Note: system IDs are zero-based (0..N-1) and refer to systems loaded via the CLI `-f`/`--file` option.

Scripting and evaluation

You can enter script expressions and statements directly at the prompt; they are parsed and executed by the embedded parser and VM (`lib/interactive/parser` and `lib/interactive/vm`). Use the `save` command to persist results to disk, or call builtins from the REPL.

Examples

- `id = 0` then `print id`
- Enter an expression such as `1 + 2 * 3` or call a builtin: `print(42)`

