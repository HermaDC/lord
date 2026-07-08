
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

> Full documentation in the [docs](docs/) folder
## Getting started
### installation

To install follow this steps

```bash
curl -O https://raw.githubusercontent.com/HermaDC/lord/refs/heads/main/install.sh
sh install.sh # launch the installing wizard, it will ask you for the version to install and if you want to install the docs
./lord-x.x.x/lord --version # to check if the installation was successful
```
You can also install the docs when the wizard ask you.

### Build 

Clone the repository and build a release or debug binary:

```bash
git clone https://github.com/HermaDC/lord
cd lord
make release       # builds dist/lord_vx.x.x
# or
make debug         # builds build/lord-debug
```

Run the release or debug binary:

```bash
./dist/lord_vx.x.x            # release
./build/debug/lord_debug       # debug
```

---

## CLI

The actual flags are:


|flag       | long flag        | description | arguments |
|-----------|------------------|-------------|-----------|
|-f         | --file           | Load system layout from a file. The file format is the same as the output of the `save` command. | filename|
|-c         | --command        | Execute a single command in non-interactive mode. The command format is the same as the commands in interactive mode. | command|
|-i         | --interactive    | Enter interactive mode. In this mode, the user can enter commands to manage the system. On exit closes the program | none|
|-u         | --update-time    | Set the update time in milliseconds for the system. This is used to simulate the passage of time in the system. | milliseconds|
|-s         | --save           | Save the current system layout to files. Each system will be saved in a separate file named `system_<id>.txt`. The file format is the same as the input file.| none|
|     /    | --verbose        | Enable verbose mode. This will print additional debug information to the console. | none |

For the reference of how to use the REPL see [docs/interactive.md](docs/interactive.md)

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


