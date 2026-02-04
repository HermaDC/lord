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

- `typedef struct Track { TrackType type; int id; Status status; struct Track* next; struct Track* prev; Direction dir; Sensor* sensors; } Track;`
  - Base structure for a track.
  - `next` and `prev` form the chain; they may be `NULL` if sides are not required.

- `typedef struct { Track base; Track* branch; SwitchPosition pos; } Switch;`
  - Extends `Track` with a `branch` and a `pos`.

---

## Public Functions

All functions are declared in `utils.h`.

### Track* create_track(int id, Sensor* sensor, Track* next, Track* prev)
- Creates and returns a new `Track` (type `STRAIGHT`).
- Parameters:
  - `id`: track identifier.
  - `sensor`: pointer to `Sensor` (owned by the track; freed in `free_tracks`).
  - `next`, `prev`: initial pointers to other `Track` (may be `NULL`).
- Return: pointer to the created `Track` or `NULL` on error (malloc).
- Notes: the `Track` initializes `status = CLEAR` and `dir = NEXT`.

---

### Switch* create_switch(int id, Sensor* sensor, Track* next, Track* prev, Track* branch)
- Creates a `Switch` (structure containing `Track base` + `branch` + `pos`).
- Parameters: similar to `create_track`, plus `branch` (pointer to the branch).
- Return: pointer to the `Switch` or `NULL` on error (malloc).
- Notes: `pos` starts at `STRAIGHT_POS`.

---

### Switch* insert_switch(Track* track_prev, Track* track_next, Sensor* sensor, Track* branch)
- Inserts a `Switch` between `track_prev` and `track_next`.
- Parameters:
  - `track_prev`, `track_next`: must be non-NULL and consecutive in the chain.
  - `sensor`: sensor of the switch (may be `NULL` if unused).
  - `branch`: points to the switch branch (may be `NULL`).
- Return: pointer to the inserted `Switch` or `NULL` if `track_prev`/`track_next` are `NULL` or on memory failure.
- Effect: updates `track_prev->next` and `track_next->prev` to link the switch.
- Safety note: ensure `branch` does not introduce unexpected cycles.

---

### Track* create_straight_line(int num_tracks)
- Creates a chain of `num_tracks` connected `Track` objects (each with its own `Sensor`).
- Return: pointer to the head of the list or `NULL` on error.
- Memory note: each `Sensor` is allocated and owned by its `Track`.

---

### int count_branch_tracks(Track* branch)
- Counts the number of tracks in a branch, including recursive sub-branches of `Switch`.
- Return: count (>= 0).

---

### int read_sensor_data(Sensor* sensor)
- Do not use in production debug builds!
- Simulation function that fills `sensor->actual_state` with a random value {0,1,2}.
- Return: the state value (0=CLEAR, 1=OCCUPIED, 2=WARNING) or `-1` if `sensor == NULL`.
- Note: the current implementation calls `srand(time(NULL))` on every call (see recommendations below).

---

### int update_track_status(Track* track)
- Reads the associated sensor and updates `track->status` accordingly.
- Behavior:
  - If `track == NULL` or `track->sensors == NULL` => returns `-1`.
  - `sensor_state < 0`: marks `track` as `OCCUPIED`, marks `track->prev` as `WARNING` if present, returns `-1`.
  - `0` => `CLEAR`, returns `0`.
  - `1` => `OCCUPIED`, marks `track->prev` as `WARNING` (if present and not `OCCUPIED`), returns `1`.
  - `2` => `WARNING`, returns `0`.
- Return: `-1` on error, `0` if CLEAR/WARNING, `1` if OCCUPIED.
- Safety recommendation: always verify `track` and `track->sensors` are not `NULL` before calling.

---

### int force_update_track_status(Track* track, Status new_status)
- Forces the `track` state to `new_status` without reading the sensor.
- Return: `0` on success, `-1` if `track == NULL`.
- Note: this function performs no additional checks; use with caution.

---

### bool is_in_chain(Track* head, Track* target)
- Checks whether `target` appears in the chain starting at `head` following `next`.
- Return: `true` if present; `false` otherwise.

---

### void free_tracks(Track* head, Track* original)
- Recursively frees a list of `Track` starting at `head` and their `Sensor` objects.
- Parameters:
  - `original`: used to avoid freeing branches that are part of the main list (pass `NULL` to use `head` as `original`).
- Notes: frees `Switch` objects and their `branch` appropriately (if not in the original chain).
- Recommendation: after `free_tracks`, do not use freed `Track` or `Sensor` pointers.

---

## Minimal Usage Example
```c
// Create a line and update the status of a track
Track* line = create_straight_line(5);
update_track_status(line->next->next->next); // updates track 4 (if it exists)
print_tracks_with_switches(line);
free_tracks(line, NULL);
```
