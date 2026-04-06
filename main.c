#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>

#include "lib/config.h"
#include "lib/utils.h"
#include "lib/cli.h"
#include "lib/types.h"
#include "lib/parser.h"

int interactive_main_loop(void){
    char *line = NULL;
    size_t len = 0;
    int status = 0;
    printf("Entering interactive mode, version %s. \nType 'help' for commands, 'exit' to quit.\n", VERSION);
    while(status == 0){
        printf(">>> "); //prompt
        if(getline(&line, &len, stdin) == -1) {
            printf("Error reading input. Exiting interactive mode.\n");
            free(line);
            return -1;
        }
        line[strcspn(line, "\n")] = 0; //remove newline
        if(strlen(line) < 1) continue;
        //TODO: parse and execute command
        if(strcmp(line, "exit") == 0) {
            printf("Exiting interactive mode\n");
            break;
        }
        if(strcmp(line, "help") == 0) {
            printf("Nothing to show\n");
            continue;
        }
        printf("Received command: %s\n", line);
    }
    return 0;
}

/*
TODO:
    -h --help show help and exit
    -i --interactive runs in interactive mode
        print prints the actual layout
        prints or print_simplified show the number of lines
        update_system updates de whole system or only a line
        list shows all the system that are currently running
        help show all the list of command with a simple definition
    -f --file the file for the initial layout
    -c --command runs the command and exits
*/ 

void msleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;              // segundos
    ts.tv_nsec = (ms % 1000) * 1000000; // nanosegundos
    nanosleep(&ts, NULL);
}

System *systems;
size_t count;

/* ---------- MAIN ---------- */

int main(int argc, char *argv[]) {
    srand(SEED);
    CLIOptions opts = parse_args(argc, argv);

    if (opts.help) {
        print_help();
        return 0;
    }

    if (opts.version) {
        printf("Version: %s (%s)[GCC %s] on Linux\n", VERSION, __DATE__, __VERSION__);
        return 0;
    }

    if (opts.verbose) {
        printf("[DEBUG] Verbose mode activated\n");
    }

    if (opts.file) {
        printf("Loaded file: %s\n", opts.file);
        systems = load_system_layout_from_file(opts.file, &count);
        printf("Loaded %zu systems from file\n", count);
        save_system_to_file(&systems[0], "output_layout.txt");

    }

    if (opts.command) {
        printf("Running: %s\n", opts.command);
        //return 0;
    }

    if (opts.interactive) {
        // loop interactivo aquí
        int err = interactive_main_loop();
        return err;

    }

    if (opts.update_time) {
        while(1){
            for(size_t i = 0; i < count; i++){
                update_system_status(&systems[i], 0);
            }
            msleep(opts.update_time);
        }
    }

    return 0;
    
}
