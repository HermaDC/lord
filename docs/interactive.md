# Interactive mode commands

To enter interactive mode, run `lord -i` or `lord --interactive` and then in the interactive shell use the following commands. the id starts from 0 and goes up to the number of systems loaded 

## List of commands

### list

Lists all the systems available. Format is `system <id>:  count <track_count>`. The sw also counts as a track.

### print \<id>

Prints the system with the given id with colored lines

### save \<id> \<filename>

Saves the system with the given id to a file with the given name. The format is the same as the input file.

### update \<id>

Updates the system with the given id.

## set \<var> \<value>

Sets a variable to a value. Then variable can be used in the commands with the syntax `$var`. For example, if you set `set id 0`, you can use `print $id` to print the system with id 0. This is useful to avoid typing the id multiple times.

### clear

Clears the screen

### help

Shows all available commands

### exit 

Closes the actual session
