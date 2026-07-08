# Builtins

The embedded language exposes a small set of builtins for output, control flow, and system inspection.

## Output and control

### print(value)

Prints a single value to standard output and returns no value.

- value: a number or boolean value
- returns: none

### exit([code])

Stops the current script execution. If no argument is provided, the script exits with code 0.

- code: optional exit code
- returns: none

## System inspection

### print_layout(id)

Prints the layout of the loaded system at the given index.

- id: zero-based system index
- returns: none

### update_system(id)

Refreshes the state of the system at the given index.

- id: zero-based system index
- returns: none

### systems_loaded()

Returns the number of systems currently loaded in the runtime context.

- returns: the current number of loaded systems
