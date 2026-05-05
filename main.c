#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "lib/cli.h"
#include "lib/config.h"
#include "lib/interactive.h"
#include "lib/parser.h"
#include "lib/types.h"
#include "lib/utils.h"

void msleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;              // segundos
    ts.tv_nsec = (ms % 1000) * 1000000; // nanosegundos
    nanosleep(&ts, NULL);
}

AppContext app_context = {
    .count = 0,
    .systems = NULL,
    .global_config = {.MAX_STACK_AMOUNT = MAX_STACK_SIZE, .VERBOSE = false}};

void cleanup(AppContext context) {
    if(!context.systems) return;
    for(size_t i = 0; i < context.count; i++) {
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
    int exit_code = 0; // Use it for storing the program exit code

    if(opts.help) {
        print_help();
        return 0;
    }

    if(opts.version) {
        printf("Version: %s (%s)[GCC %s] on Linux\n", VERSION, __DATE__, __VERSION__);
        return 0;
    }

    if(opts.verbose) {
        printf("[DEBUG] Verbose mode activated\n");
        app_context.global_config.VERBOSE = true;
    }

    if(opts.file) {
        printf("Loaded file: %s\n", opts.file);
        app_context.systems = load_system_layout_from_file(opts.file, &app_context.count);
        if(!app_context.systems) {
            fprintf(stderr, "Error: Failed to load system layout from file: %s\n",
                    opts.file);
            return 3;
        }
        printf("Loaded %zu systems from file\n", app_context.count);
    }

    if(opts.command) {
        printf("Running: %s\n", opts.command);
        int err = run_command_cli(opts.command);

        exit_code = err;
        goto on_exit;
    }

    if(opts.interactive) {
        if(opts.update_time) {
            fprintf(stderr, "Update time set and interactive too\n aborting...");
            return 1;
        }
        int err = interactive_main_loop();
        if(err != CMD_ERR_OK) {
            exit_code = 2;
            goto on_exit;
        }
        exit_code = err;
        goto on_exit;
    }

    if(opts.update_time) {
        if(opts.interactive) {
            fprintf(stderr, "Update time set and interactive too\nAborting...");
            return 1;
        }
        while(1) {
            if(opts.update_time) {
                fprintf(stderr, "Update time set and interactive too\n aborting...");
                return 3;
            }
            // loop interactivo aquí
            int err = interactive_main_loop();
            return err;

            for(size_t i = 0; i < app_context.count; i++) {
                update_system_status(&app_context.systems[i], 0);
            }
            msleep(opts.update_time);
        }
    }
    if(opts.save) {
        for(size_t i = 0; i < app_context.count; i++) {
            char filename[256];
            snprintf(filename, sizeof(filename), "system_%zu.txt", i);
            ErrorCode err = save_system_to_file(&app_context.systems[i], filename);
            if(err != ERR_OK) {
                fprintf(stderr, "Error: Failed to save system %zu to file %s\n", i,
                        filename);
                exit_code = 3;
                goto on_exit;
            }
        }
    }
    if(opts.script) {
        FILE *script_file = fopen(opts.script, "r");
        if(!script_file) {
            fprintf(stderr, RED "Error: " RESET "Could not open script file: %s\n",
                    opts.script);
            exit_code = 2;
            goto on_exit;
        }
        int err = run_script_file(script_file);
        fclose(script_file);
        exit_code = err;
        goto on_exit;
    }
    // TODO use directyly goto on_exit, as it is the last instruction of the function
    cleanup(app_context);
    return exit_code;

on_exit:
    cleanup(app_context);
    return exit_code;
}
