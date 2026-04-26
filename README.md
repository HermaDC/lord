
# LORD

A small C project to model and manage a simple railway system (tracks, switches and sensors) for learning and testing purposes.

---

## Features 

- Manage linked tracks and switches (create, insert, and free).
- Simulated sensors and utilities to update track status.
- Helpers to build straight lines and count branch tracks.
- Minimal, portable C code with a Makefile for debug and release builds.
- Basic logging of creation and states.

---

## Requirements 

- gcc (tested with GCC 9+)
- make

---

## Build & Run 

Clone the repository and build a release or debug binary:

```bash
git clone https://github.com/HermaDC/lord
cd lord
make release       # builds dist/lord_v0.0.1
# or
make debug         # builds debug-datetime
```

Run the release or debug binary:

```bash
./dist/lord_v0.0.1    # release
./debug-date--time              # debug
```

> Note: Use `make run-release` or `make run-debug` to build and run in one step.

---

## Usage / Example

Minimal example showing how to create a line and update a track state:

```c
System sys;
init_system(&sys, 10); // Initialize system with capacity for 10 tracks
create_straight_line(&sys, 10, NULL); // Create a line of 10 tracks
update_system_status(&sys, 2); // Update the system from index 2 to the correspondingly state
force_update_track_status(&sys, 3, OCCUPIED); // Force update the track at index 3 to state 1 (occupied)
free_system(&sys); // Free system resources
```

Another example of how to load the layout from a file:

```c
size_t count;
System *heads = load_system_layout_from_file("./layout", &count); //returns a array of all the heads in the file
for(int i = 0; i < count; i++){
    print_tracks_with_switches(&heads[i], 0); //print the layout with the head[i] as root
    free_system(&heads[i]); //free the track with head[i]
    printf("\n");
}
free(heads); //free the array of heads    
```
## CLI

The actual flags are:


flag       | long flag        | description | arguments
-----------|------------------|-------------|-----
-f         | --file           | Load system layout from a file. The file format is the same as the output of the `save` command. | filename
-c         | --command        | Execute a single command in non-interactive mode. The command format is the same as the commands in interactive mode. | command
-i         | --interactive    | Enter interactive mode. In this mode, the user can enter commands to manage the system. On exit closes the program | none
-u         | --update-time    | Set the update time in milliseconds for the system. This is used to simulate the passage of time in the system. | milliseconds
-s         | --save           | Save the current system layout to files. Each system will be saved in a separate file named `system_<id>.txt`. The file format is the same as the input file.| none
-v         | --verbose        | Enable verbose mode. This will print additional debug information to the console. | none

For the list of commands refer to [docs/interactive.md](docs/interactive.md)

> note: Interactive and update time cannot be run together, returns exit code 1.

## Exit codes

    Code | Description
    -----|-------------
    0    | Success
    1    | General error (e.g., invalid arguments)
    2    | error in interactive/scripting mode or with --command
    3    | error loading or saving layout from/to file

> Many of this errors will be written in the log file with a description of the error.



## Contributing 

- Please open issues or pull requests for bugs or feature requests.
- Keep changes small and well-documented.

---


