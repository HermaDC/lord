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

- `typedef struct Track { TrackType type; int id; Status status; struct Track *next; struct Track *prev; Direction dir; Sensor *sensors; } Track;`
  - Base structure for a track.
  - `next` and `prev` form the chain; they may be `NULL` if sides are not required.

- `typedef struct { Track base; Track *branch; SwitchPosition pos; } Switch;`
  - Extends `Track` with a `branch` and a `pos`.

- `typedef struct { int top; Switch *data[MAX_STACK_SIZE];} SwitchStack;`
  - Creates a stack of Switch pointers

- `typedef enum { TOKENIZE_OK, TOKENIZE_MISSING_NUM, TOKENIZE_UNKNOWN_CHAR,    TOKENIZE_UNMATCHED_PARENTHESES, TOKENIZE_OOM, TOKENIZE_EMPTY_STR } TokenizeError;`
  - Stores the error of the tokenize, used for debugging and printing errors.

- `typedef enum { NUMBER, OPEN, CLOSE, SW } TokenType;`
  - Represents the type of token.

- `typedef struct {int value; TokenType type; size_t column;} Token;`
  - The token structure, holds the type value and the column of the string where the token cames from.

---

## Public Functions

### Track \*create_track(int id, Sensor *sensor, Track *next, Track *prev)
- Creates and returns a new `Track` (type `STRAIGHT`).
- Parameters:
  - `id`: track identifier.
  - `sensor`: pointer to `Sensor` (owned by the track; freed in `free_tracks`).
  - `next`, `prev`: initial pointers to other `Track` (may be `NULL`).
- Return: pointer to the created `Track` or `NULL` on error (malloc).
- Notes: the `Track` initializes `status = CLEAR` and `dir = NEXT`.

---

### Switch *create_switch(int id, Sensor *sensor, Track *next, Track *prev, Track *branch)
- Creates a `Switch` (structure containing `Track base` + `branch` + `pos`).
- Parameters: similar to `create_track`, plus `branch` (pointer to the branch).
- Return: pointer to the `Switch` or `NULL` on error (malloc).
- Notes: `pos` starts at `STRAIGHT_POS`.

---

### Switch *insert_switch(Track *track_prev, Track *track_next, Sensor *sensor, Track *branch)
- Inserts a `Switch` between `track_prev` and `track_next`.
- Parameters:
  - `track_prev`, `track_next`: must be non-NULL and **consecutive** in the chain.
  - `sensor`: sensor of the switch (may be `NULL` if unused).
  - `branch`: points to the switch branch (may be `NULL`).
- Return: pointer to the inserted `Switch` or `NULL` if `track_prev`/`track_next` are `NULL` or on memory failure.
- Effect: updates `track_prev->next` and `track_next->prev` to link the switch.
- Safety note: ensure `branch` does not introduce unexpected cycles.

---

### Track *create_straight_line(int num_tracks)
- Creates a chain of `num_tracks` connected `Track` objects (each with its own `Sensor`).
- Return: pointer to the head of the list or `NULL` on error.
- Memory note: each `Sensor` is allocated and owned by its `Track`.

---

### int count_branch_tracks(Track *branch)
- Counts the number of tracks in a branch, including recursive sub-branches of `Switch`.
- Return: count (>= 0).

---

### int read_sensor_data(Sensor *sensor)
- :warning: Do not use in production debug builds!
- Simulation function that fills `sensor->actual_state` with a random value {0,1,2}.
- Return: the state value (0=CLEAR, 1=OCCUPIED, 2=WARNING) or `-1` if `sensor == NULL`.

---

### int update_track_status(Track *track)
- Reads the associated sensor and updates `track->status` accordingly.
- Behavior:
  - If `track == NULL` or `track->sensors == NULL` => returns `-1`.
  - `sensor_state < 0`: marks `track` as `OCCUPIED`, marks `track->prev` or `track->next` (depending on `track->dir`) as `WARNING` if present, returns `-1`.
  - `0` => `CLEAR`, returns `0`.
  - `1` => `OCCUPIED`, marks `track->prev` or `track->next` (depending on `track->dir`) as `WARNING` (if present and not `OCCUPIED`), returns `1`.
  - `2` => `WARNING`, returns `0`.
- Return: `-1` on error, `0` if CLEAR/WARNING, `1` if OCCUPIED.
- Safety recommendation: always verify `track` and `track->sensors` are not `NULL` before calling.

---

### void update_system_status(Track *head)
- Updates the status of all tracks in a chain starting from `head` by reading their sensors.
- Parameters:
  - `head`: pointer to the first track (may be `NULL` to do nothing).
- Effect: calls `update_track_status()` on each track following `next` pointers. Does not traverse branches of `Switch` objects.
- Notes: safe to call with `NULL` head.

---

### Track *get_last_track(Track *head)
- Traverses the chain from `head` and returns the last track (where `next == NULL`).
- Return: pointer to the last track, or `head` if it's the only track, or `NULL` if `head == NULL`.
- Notes: does not traverse `Switch` branches.

---

### int force_update_track_status(Track *track, Status new_status)
- Forces the `track` state to `new_status` without reading the sensor.
- Return: `0` on success, `-1` if `track == NULL`.
- Note: this function performs no additional checks; use with caution.

---

### bool is_in_chain(Track *head, Track *target)
- Checks whether `target` appears in the chain starting at `head` following `next`.
- Return: `true` if present; `false` otherwise.

---

### Track **load_system_layout_from_file(const char *path, size_t *count)
- Loads the system layout from the file in `path`
- Parameters:
  - `path`: file path to read (must be a valid file).
  - `count`: pointer to `size_t` where the number of lines loaded will be stored.
- Return: pointer to an array of `Track*` heads (one for each line in the file) or `NULL` on error (invalid path, file format, memory failure).
- Free the pointer returned. The tracks should be freed with their function [`free_tracks()`](#void-free_trackstrack-head-track-original)


### void free_tracks(Track *head, Track *original)
- Recursively frees a list of `Track` starting at `head` and their `Sensor` objects.
- Parameters:
  - `original`: used to avoid freeing branches that are part of the main list (pass `NULL` to use `head` as `original`).
- Notes: frees `Switch` objects and their `branch` appropriately (if not in the original chain).
- Recommendation: after `free_tracks`, do not use freed `Track` or `Sensor` pointers.

---

## Utility Functions (main.c)

### Track *get_next_track(Track *track)
- Returns the next track logically following `track`.
- Behavior:
  - If `track` is a `Switch` in `DIVERGING_POS` with a branch, returns the branch.
  - Otherwise, returns `track->next` (may be `NULL`).
- Return: pointer to the next track or `NULL` if there is none.
- Notes: useful for traversing the system while respecting switch positions.

---

### void print_tracks_with_switches(Track *head)
- Prints a visual representation of all tracks starting from `head`, including switches and branches.
- Output format:
  - Regular tracks are shown as `------ ` (with color coding: green=CLEAR, red=OCCUPIED, yellow=WARNING).
  - Switches display `SW[→]` (straight position) or `SW[~]` (diverging position) followed by branch count in parentheses.
  - Branches are enclosed in curly braces `{ ... }`.
- Notes: does not include newlines between chains; print a newline manually if needed.
- Parameters:
  - `head`: pointer to the first track (may be `NULL` to do nothing).

---

## Minimal Usage Example
```c
// Create a line and update the status of a track
Track *line = create_straight_line(5);
update_track_status(line->next->next->next); // updates track 4 (if it exists)
print_tracks_with_switches(line);
free_tracks(line, NULL);

// Load system from file
size_t count = 0;
Track **system = load_system_layout_from_file("./system_state.txt", &count);
if (system) {
    for (int i = 0; i < count; i++) {
        update_system_status(system[i]);
        print_tracks_with_switches(system[i]);
        printf("\n");
    }
    
    // Free all chains
    for (int i = 0; system[i]; i++) {
        free_tracks(system[i], NULL);
    }
    free(system);
}
```
