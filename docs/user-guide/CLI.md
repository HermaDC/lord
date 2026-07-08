# CLI options

## script

Lord opens the file and parser it and execute it with the integrated parser and VM. The reference for the syntax language and other related things are in [docs/language](docs/language)

## -f --file path

This option opens and reeds all the systems in the file provided by the path, then, if the syntax is correct it will load them into memory to use them, each line of the file is equivalent to a system. If a system has any syntax error, it will be reported and not loaded. 
For a full reference of the syntax of the layout config file see [docs/layout-script.md](docs/layout-script.md)


## -c --command command

This option runs one single command. This is useful in case you don't want to open the whole REPL session or for small tests. The syntax is the same as the REPL.
The command should be passed through quotation marks to avoid the shell to split the command

## -i --interactive

Opens the REPL session. The syntax is the same as in the script. The syntax, builtins and other information is in [docs/language](docs/language)

## -s --save 

> [!WARNING]
> This option is missing some implementation, now only return the same systems as -f 

This option save the systems with the name `system_<id>` in the current directory using the layout script defined in [docs/user-guide/layout-script.md](docs/user-guide/layout-script.md). 

## --verbose

With this option all the log message with the category of `DEBUG` will be log in the log file. The log file is by default `lord.log`, but can be modified in the build proccess.


