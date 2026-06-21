# API Reference — LORD

## Overview

This document describes the public structures and functions of the LORD project. It includes function signatures, parameter descriptions, return values, usage examples, and a troubleshooting section focused on possible core dumps and how to debug them.

---

## Public Types

- `typedef enum Status { CLEAR, OCCUPIED, WARNING } Status;`
  - State of a `Track`.

- `typedef enum direction { NEXT = 1, PREV = -1 } Direction;`
  - Logical direction a `Track` points to.

- `typedef struct Sensor { int hex_direction; int actual_state; } Sensor;`
  - Represents the sensor associated with a `Track`.

- `typedef enum TrackType { STRAIGHT, SWITCH_TRACK } TrackType;`
  - Track type: straight or switch.

- `typedef enum SwitchPosition { STRAIGHT_POS, DIVERGING_POS } SwitchPosition;`
  - Position of a `Switch`.

- `typedef struct Track { TrackType type; int id; Status status; int next_index; int prev_index; Direction dir; Sensor *sensors; } Track;`
  - Base structure for a track.
  - `next_index` and `prev_index` form the chain.

- `typedef enum {ERR_OK = 0, ERR_GENERAL, ERR_INVALID_ARG, ERR_NULL_PTR, ERR_NO_MEMORY, ERR_NOT_FOUND, ERR_ALREADY_EXISTS, ERR_EMPTY, ERR_INVALID_STATE, ERR_OUT_OF_BOUNDS,    ERR_NOT_CONNECTED, ERR_BROKEN_LINK} ErrorCode;`
  - General error codes for functions.

//Holds all the info for a line of the system
- `typedef struct System {Track *array; int count; size_t buffer;} System;`
  - Represents the entire system as an array of tracks, with a count of how many tracks are currently in the system and the buffer size allocated.
  - `array` is a dynamic array of `Track` structures, where each track can be a straight track or a switch track. 
  - The `count` field indicates how many tracks are currently in the system, while the `buffer` field indicates the total allocated size of the array.

- `typedef struct { int top; void *data[MAX_STACK_SIZE];} SwitchStack;`
  - Creates a stack of Switch pointers

- `typedef enum { TOKENIZE_OK, TOKENIZE_MISSING_NUM, TOKENIZE_UNKNOWN_CHAR,    TOKENIZE_UNMATCHED_PARENTHESES, TOKENIZE_OOM, TOKENIZE_EMPTY_STR } TokenizeError;`
  - Stores the error of the tokenize, used for debugging and printing errors.

- `typedef enum { NUMBER, OPEN, CLOSE, SW } TokenType;`
  - Represents the type of token.

- `typedef struct {int value; TokenType type; size_t column;} Token;`
  - The token structure, holds the type value and the column of the string where the token came from.

- `typedef struct {int help; int interactive; int version; int verbose; char *file; char *command; int update_time;} CLIOptions;`
  - Stores the options of the CLI, used for parsing the arguments and printing the help.
---

## Public Functions

### const char *error_to_string(ErrorCode err);
  - parses the error value to a string to help debugging

### CLIOptions parse_args(int argc, char *argv[]);
  - Parses the command line arguments and returns a `CLIOptions` struct with the parsed values.
  - Parameters:
    - `argc`: argument count from `main()`.
    - `argv`: argument vector from `main()`.
  - Return: a `CLIOptions` struct with the parsed options.

### void print_help();
  - Prints the help message for the command-line interface.

### ErrorCode create_track(System *system, int id, Sensor *sensor, int next, int prev);
  - Creates a track in system with the params, if id is 0 will generate one automatically

### ErrorCode create_switch(System *system, int id, Sensor *sensor, int next, int prev, int branch);
  - Creates a switch in system with the params, if id is 0 will generate one automatically

### ErrorCode insert_switch(System *system, int id, Sensor *sensor, int track_next, int track_prev, int branch);
  - Creates a switch and inserts it in system with the params, if id is 0 will generate one automatically

### ErrorCode create_straight_line(System *system, int num_tracks, size_t *index);
  - Creates a num_tracks straight line of tracks in system, in index stores the index of the first track of the line
  - Parameters:
    - `system`: pointer to the `System` struct representing the entire system.
    - `num_tracks`: number of straight tracks to create in the line.
    - `index`: pointer to a size_t variable where the index of the first track of the line will be stored.
  # LORD — API Reference

  ## Overview

  This document provides a concise reference for the public types and modules in the LORD project. It summarizes the principal data structures and the main entry points that other code or users are expected to call. For implementation details consult the headers in `lib/`.

  ## Project modules (high level)

  - **CLI**: argument parsing and top-level program flow (`lib/cli.h`, `main.c`).
  - **Layout parser**: tokenizes and parses system layout files used to build `System` instances (`lib/layout-parser.*`).
  - **Types & system model**: core data structures such as `Track`, `System`, and helper routines (`lib/types.h`).
  - **Interactive / REPL**: command-line REPL, history and completion (uses `linenoise`, `lib/interactive/`).
  - **Script parser & VM**: a small AST-based scripting language with evaluation and builtin calls (`lib/interactive/parser/*`, `lib/interactive/vm/*`).

  ## Key public types (summary)

  - `System` — in-memory representation of a line/system composed of `Track` entries. See `lib/types.h` for fields and lifecycle functions.
  - `Track` — single track entry (straight or switch) with links to adjacent tracks and optional sensors.
  - `ErrorCode` — canonical error codes used across the codebase (`ERR_OK`, `ERR_INVALID_ARG`, etc.).
  - `CLIOptions` — parsed command-line options returned by `parse_args()` (see `lib/cli.h`).

  ## Principal functions and entry points

  The project exposes a small set of functions intended for consumers or tests; the implementation headers are the authoritative source.

  - `CLIOptions parse_args(int argc, char *argv[])` — parse command-line arguments (`lib/cli.h`).
  - `void print_help(void)` — prints command-line usage (`lib/cli.h`).
  - `System *load_system_layout_from_file(const char *path, size_t *count)` — parse a layout file and return an array of `System` objects.
  - `ErrorCode save_system_to_file(System *system, const char *path)` — persist a `System` to disk.
  - `ErrorCode init_system(System *sys, size_t initial_capacity)` / `void free_system(System *sys)` — lifecycle helpers for `System` objects.
  - `int run_script_file(FILE *f)` — parse and execute a script file using the embedded VM (`lib/interactive/interactive.c`).
  - `int run_interactive_loop(void)` — start the interactive REPL (history & completion) (`lib/interactive/interactive.c`).

  ## Documentation notes

  - This file is a top-level quick reference. For full API and field-level documentation, open the corresponding header files (for example `lib/types.h`, `lib/cli.h`, `lib/layout-parser.h`, and files under `lib/interactive/`).
  - The scripting language and VM are intentionally small and are implemented under `lib/interactive/parser` and `lib/interactive/vm`.

  If you want, I can add cross-references (links) from these summary entries to specific header line numbers in the repository.
  - Initializes the system with an initial capacity for tracks.
