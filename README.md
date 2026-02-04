
# gestion-trenes

A small C project to model and manage a simple railway system (tracks, switches and sensors) for learning and testing purposes.

---

## Features 

- Manage linked tracks and switches (create, insert, and free).
- Simulated sensors and utilities to update track status.
- Helpers to build straight lines and count branch tracks.
- Minimal, portable C code with a Makefile for debug and release builds.

---

## Requirements 

- gcc (tested with GCC 9+)
- make

---

## Build & Run 

Clone the repository and build a release or debug binary:

```bash
git clone <repo-url>
cd gestion-trenes
make release       # builds dist/gestion_trenes_v0.0.1
# or
make debug         # builds gestion_trenes_debug
```

Run the release or debug binary:

```bash
./dist/gestion_trenes_v0.0.1    # release
./gestion_trenes_debug          # debug
```

Note: Use `make run-release` or `make run-debug` to build and run in one step.

---

## Usage / Example

Minimal example showing how to create a line and update a track state:

```c
Track* line = create_straight_line(5);
update_track_status(line->next->next->next); // update track 4 if exists
print_tracks_with_switches(line);
free_tracks(line, NULL);
```

---

## Documentation 

The API reference is in `docs/API_en.md` (English). For more details on structures, functions, and examples, consult that file.

---

## Contributing 

- Please open issues or pull requests for bugs or feature requests.
- Keep changes small and well-documented.

---


