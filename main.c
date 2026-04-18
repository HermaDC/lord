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
#include "lib/interactive.h"


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


AppContext app_context = {
    .count = 0, 
    .systems = NULL, 
    .global_config = {
        .MAX_STACK_AMOUNT = MAX_STACK_SIZE,
        .VERBOSE = false
    }
};

void cleanup(AppContext context) {
    if(!context.systems) return;
    for (size_t i = 0; i < context.count; i++) {
        free_system(&context.systems[i]);
    }
    free(context.systems);
    context.systems = NULL;
    context.count = 0;
}

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
        app_context.global_config.VERBOSE = true;
    }

    if (opts.file) {
        printf("Loaded file: %s\n", opts.file);
        app_context.systems = load_system_layout_from_file(opts.file, &app_context.count);
        printf("Loaded %zu systems from file\n", app_context.count);

    }

    if (opts.command) {
        printf("Running: %s\n", opts.command);
        //return 0;
    }

    if (opts.interactive) {
        if(opts.update_time){
            fprintf(stderr, "Update time set and interactive too\n aborting...");
            return 3;
        }
        // loop interactivo aquí
        int err = interactive_main_loop();
        cleanup(app_context);
        return err;

    }

    if (opts.update_time) {
        if(opts.interactive){
            fprintf(stderr, "Update time set and interactive too\nAborting...");
            return 3;
        }
        while(1){
        if(opts.update_time){
            fprintf(stderr, "Update time set and interactive too\n aborting...");
            return 3;
        }
        // loop interactivo aquí
        int err = interactive_main_loop();
        return err;

    
            for(size_t i = 0; i < app_context.count; i++){
                update_system_status(&app_context.systems[i], 0);
            }
            msleep(opts.update_time);
        }
    }
    if (opts.save) {
        for(size_t i = 0; i < app_context.count; i++){
            char filename[256];
            snprintf(filename, sizeof(filename), "system_%zu.txt", i);
            save_system_to_file(&app_context.systems[i], filename);
        }
    }
    if (opts.script){
        printf("script %s", opts.script);
    }
    cleanup(app_context);
    return 0;
    
}
