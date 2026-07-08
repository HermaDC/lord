# API overview

This page gives a short overview of the main types and entry points in LORD. It is intended as a practical reference for using the core library, the CLI, and the embedded scripting interface without going into every internal helper.

## Project modules (structural overview)

The project is organized into modular components, each responsible for a specific domain:

- **CLI** (`lib/cli.c/h`, `main.c`): handles command-line argument parsing and top-level program flow. Entry point for all execution modes (file loading, interactive mode, script execution).

- **Core types** (`lib/types.c/h`): defines the fundamental data structures (`System`, `Track`, `Status`, `Direction`) and their lifecycle functions (`init_system()`, `free_system()`).

- **Layout parser** (`lib/layout_parser.c/h`): tokenizes and parses system layout files using a simple syntax (numbers, parentheses, switches). Produces `System` instances from disk.

- **Utils** (`lib/utils.c/h`): general-purpose helpers for system manipulation (track creation, state updates) and file I/O (load/save layout files).

- **Interactive / REPL** (`lib/interactive/interactive.c/h`): provides the command-line REPL loop with history and completion (uses linenoise). Entry point for interactive scripts and scripting.

- **Script parser** (`lib/interactive/parser/`): lexer and parser for the embedded scripting language. Produces an AST from script source.

- **VM / Evaluator** (`lib/interactive/vm/`): evaluates AST nodes, manages variables and stack, and dispatches builtin function calls.

- **Linenoise** (`lib/linenoise-lib/`): third-party library providing line editing and history support.

## Core types

- System: the main in-memory container for a track layout. It holds the array of tracks and their current count.
- Track: a single track entry in the system. A track can be straight or a switch and may carry links to neighboring tracks and optional sensors.
- Status, Direction, and SwitchPosition: small enums that describe the state of a track and the orientation of a switch.
- ErrorCode: the shared error type returned by most public routines.

## Main lifecycle functions

- init_system(): prepares a System for use.
- free_system(): releases all resources owned by a System.
- create_track(), create_switch(), and insert_switch(): create and link tracks or switches inside a system.
- create_straight_line(): builds a simple linear sequence of tracks.
- force_update_track_status() and update_system_status(): update track state and propagate changes through the system.

## File and CLI helpers

- parse_args(): parses command-line options such as file input, interactive mode, and script execution.
- print_help(): prints the CLI usage summary.
- load_system_layout_from_file(): loads a layout from disk into one or more System objects.
- save_system_to_file(): saves a System to disk.

## Interactive and scripting entry points

- run_script_file(): executes a script from a file.
- run_command_line(): runs a single command string.
- run_interactive_loop(): starts the REPL.

The reference intentionally omits low-level parser and VM helpers that are only relevant to implementation work inside the lib/ tree.
