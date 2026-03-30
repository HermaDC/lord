# API Reference — gestion-trenes 

## Overview

This document describes the public structures and functions of the `gestion-trenes` (C) project. It includes function signatures, parameter descriptions, return values, usage examples, and a troubleshooting section focused on possible core dumps and how to debug them.

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
  - The token structure, holds the type value and the column of the string where the token cames from.

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
  - Return: `ERR_OK` on success, or an appropriate error code on failure.

### int get_last_track(System *system, int start_index);
  - Returns the index of the last track connected counting from start_index
  - Parameters:
    - `system`: pointer to the `System` struct representing the entire system.
    - `start_index`: index of the track to start counting from.
  - Return: index of the last track.


### int get_next_track(System *system, int index);
  - Returns the index of the next track from index
  - Parameters:
    - `system`: pointer to the `System` struct representing the entire system.
    - `index`: index of the track to get the next track from.
  - Return: index of the next track. 

### int count_branch_tracks(System *system, int branch_index);
  - Counts recursively the tracks, including the branches, counting from branch_index
  - Parameters:
    - `system`: pointer to the `System` struct representing the entire system.
    - `branch_index`: index of the track to start counting from (should be a branch track).
  - Return: total count of tracks in the branch.

### ErrorCode force_update_track_status(System *system, int track_index, Status new_status);
  - Forces to update the track_index track to new_status
  - Parameters:
    - `system`: pointer to the `System` struct representing the entire system.
    - `track_index`: index of the track to update.
    - `new_status`: new status to set for the track.
  - Return: `ERR_OK` on success, or an appropriate error code on failure.

### void update_system_status(System *system, int index);
  - Updates all the system
  - Parameters:
    - `system`: pointer to the `System` struct representing the entire system.
    - `index`: index of the track to start the update from (can be any track in the system).

### ErrorCode init_system(System *sys, size_t initial_capacity);
  - Initializes the system with an initial capacity for tracks.
  - Parameters:
    - `sys`: pointer to a `System` struct to initialize.
    - `initial_capacity`: initial number of tracks to allocate space for.
  - Return: `ERR_OK` on success, or an appropriate error code on failure.
---

void print_tracks_with_switches(System *system, int index);
  - Prints the system tracks with the switches, starting from index
  - Parameters:
    - `system`: pointer to the `System` struct representing the entire system.
    - `index`: index of the track to start printing from (can be any track in the system).

---

## Minimal Usage Example
```c
int main(){
    //Creates and initialize the system
    System sys;
    init_system(&sys, 10);

    size_t head_index;

    //creates a straight line of 5 tracks
    ErrorCode err = create_straight_line(&sys, 5, &head_index);
    if (err != ERR_OK) {
        fprintf(stderr, "Error creating straight line\n");
        return 1;
    }

    //Prints before and after updating the system status
    print_tracks_with_switches(&sys, 0);
    printf("\n");
    update_system_status(&sys, 0);
    print_tracks_with_switches(&sys, 0);
    return 0;
}
```
