#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cli/cli.h"
#include "common/config.h"
#include "core/layout-parser.h"
#include "runtime/socket.h"
#include "core/types.h"
#include "core/utils.h"

void msleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;              // segundos
    ts.tv_nsec = (ms % 1000) * 1000000; // nanosegundos
    nanosleep(&ts, NULL);
}

AppContext app_context = {.count = 0, .systems = NULL};

struct Config global_config = {.MAX_STACK_AMOUNT = MAX_STACK_SIZE, .VERBOSE = false};

void cleanup(void) {
    if(!app_context.systems) return;
    for(size_t i = 0; i < app_context.count; i++) {
        free_system(&app_context.systems[i]);
    }
    free(app_context.systems);
    app_context.systems = NULL;
    app_context.count = 0;
    return;
}

/* ---------- MAIN ---------- */

int main(int argc, char *argv[]) {
    atexit(cleanup);
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
        global_config.VERBOSE = true;
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
        VM vm = make_VM();
        int err = run_command_line(&vm, opts.command);
        destroy_VM(&vm);

        exit_code = err;
        goto on_exit;
    }

    if(opts.interactive) {
        if(opts.update_time) {
            fprintf(stderr, "Update time set and interactive too\n aborting...");
            return 1;
        }
        int err = run_interactive_loop();
        exit_code = err;
        goto on_exit;
    }

    if(opts.update_time) {

        IPCServer server = {0};
        int server_fd = init_socket("/tmp/lord.sock");
        if(server_fd < 0) return 10;
        server.server_fd = server_fd;
        while(1) {
            if(opts.interactive) {
                fprintf(stderr, "Update time set and interactive too\n aborting...");
                return 3;
            }
            ipc_poll(&server);
            for(size_t i = 0; i < app_context.count; i++) {
                update_system_status(&app_context.systems[i], 0);
                for(size_t i = 0; i < server.client_count; i++) {
                    send_message(server.clients[i].fd, MSG_EVENT,
                                 "Chanvales, se ha actualizado\n");
                }
            }

            msleep(opts.update_time);
        }
    }
    if(opts.save) {
        save_system_to_json(&app_context.systems[0], "prueba.json");
        return 0;
    }
    /*
    if(opts.save) {
#warning "feature not implemented, may not use"
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
    }*/
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
    return exit_code;

on_exit:
    return exit_code;
}
