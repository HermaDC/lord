#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "interactive.h"
#include "types.h"
#include "utils.h"

#include "linenoise-lib/linenoise.h"

#define TOKEN_CHUNK 64

// TODO add enum for for command exit status

typedef int (*command_func)(char **args);

typedef struct {
    const char *name;
    command_func func;
    int min_args;
    const char *args_str;
    const char *usage;
    const char *desc;
} Command;

static char **parse_input(char *input) {
    if(!input) return NULL;
    int capacity = TOKEN_CHUNK;
    int count = 0;

    char **tokens = malloc(capacity * sizeof(char *));
    if(!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    char *token = strtok(input, " ");
    while(token) {
        if(count >= capacity) {
            capacity += TOKEN_CHUNK;
            tokens = realloc(tokens, capacity * sizeof(char *));
            if(!tokens) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        tokens[count++] = token;
        token = strtok(NULL, " ");
    }

    tokens[count] = NULL;
    return tokens;
}

static inline int count_args(char **args) {
    int i = 0;
    while(args[i])
        i++;
    return i;
}

CmdErrorCode command_show_help(char **args);
CmdErrorCode command_list_systems(char **args);
CmdErrorCode command_print_system(char **args);
CmdErrorCode command_save_system(char **args);
CmdErrorCode command_update_system(char **args);
CmdErrorCode command_clear_screen(char **args);

static const Command commands[] = {
    {"help", command_show_help, 0, NULL, "help", "Show this help"},
    {"list", command_list_systems, 0, NULL, "list", "List all systems"},
    {"print", command_print_system, 1, " <id>", "print <id>", "Print a system"},
    {"save", command_save_system, 2, " <id> <name>", "save <id> <name>", "Save a system"},
    {"update", command_update_system, 1, " <id>", "update <id>", "Update a system"},
    {"clear", command_clear_screen, 0, NULL, "clear", "Clear the screen"},
    {NULL, NULL, 0, NULL, NULL, NULL}};

static int execute_command(char **args) {
    if(!args || !args[0]) return -1;

    int argc = count_args(args) - 1;

    for(size_t i = 0; commands[i].name != NULL; i++) {
        if(strcmp(args[0], commands[i].name) == 0) {

            if(argc < commands[i].min_args) {
                fprintf(stderr, "Command '%s' needs %d args, got %d\n", commands[i].name,
                        commands[i].min_args, argc);
                return -1;
            }

            return commands[i].func(args);
        }
    }

    fprintf(stderr, "Unknown command: «%s». Try «help» to get a list of commands\n",
            args[0]);
    return -1;
}

CmdErrorCode command_show_help(char **args) {
    (void) args;

    printf("Available commands:\n");

    for(size_t i = 0; commands[i].name != NULL; i++) {
        printf("  %-15s - %s\n", commands[i].usage, commands[i].desc);
    }
    printf("  %-15s - %s\n", "exit", "Exits the interactive session");

    return 0;
}

CmdErrorCode command_list_systems(char **args) {
    (void) args;
    if(app_context.count == 0) {
        printf("No systems loaded\n");
        return CMD_ERR_OK;
    }
    for(size_t i = 0; i < app_context.count; i++) {
        printf("System %zu: count %d\n", i, app_context.systems[i].count);
    }
    return CMD_ERR_OK;
}

CmdErrorCode command_print_system(char **args) {
    char *endptr;
    int id = (int) strtol(args[1], &endptr, 10);
    if(*endptr != '\0') {
        fprintf(stderr, "Invalid system id: %s\n", args[1]);
        return CMD_ERR_INVALID_ARG;
    }
    if(id < 0 || id >= (int) app_context.count) {
        fprintf(stderr, "Invalid system id: %d. Maximum is %d\n", id,
                (int) app_context.count - 1);
        return CMD_ERR_INVALID_ARG;
    }
    print_tracks_with_switches(&app_context.systems[id], 0);
    printf("\n");
    return CMD_ERR_OK;
}

CmdErrorCode command_save_system(char **args) {
    printf("Saving system: %s\n", args[1]);
    char *endptr;
    int id = (int) strtol(args[1], &endptr, 10);
    if(*endptr != '\0') {
        fprintf(stderr, "Invalid system id: %s\n", args[1]);
        return CMD_ERR_INVALID_ARG;
    }
    if(id < 0 || id >= (int) app_context.count) {
        fprintf(stderr, "Invalid system id: %d. Maximum is %d\n", id,
                (int) app_context.count - 1);
        return CMD_ERR_INVALID_ARG;
    }
    ErrorCode err = save_system_to_file(&app_context.systems[id], args[2]);
    if(err != ERR_OK) {
        fprintf(stderr, "Failed to save system: %s\n", args[1]);
        return CMD_ERR_OK;
    }
    return CMD_ERR_OK;
}
CmdErrorCode command_clear_screen(char **args) {
    (void) args;
    linenoiseClearScreen();
    return CMD_ERR_OK;
}

CmdErrorCode command_update_system(char **args) {
    char *endptr;
    int id = (int) strtol(args[1], &endptr, 10);
    if(*endptr != '\0') {
        fprintf(stderr, "Invalid system id: %s\n", args[1]);
        return CMD_ERR_INVALID_ARG;
    }
    if(id < 0 || id >= (int) app_context.count) {
        fprintf(stderr, "Invalid system id: %d. Maximum is %d\n", id,
                (int) app_context.count - 1);
        return CMD_ERR_INVALID_ARG;
    }
    update_system_status(&app_context.systems[id], 0);
    return 0;
}

// TODO make a loop or a proper tree of predictions
void completion(const char *buf, linenoiseCompletions *lc) {
    for(size_t i = 0; commands[i].name != NULL; i++) {
        if(strncmp(buf, commands[i].name, strlen(buf)) == 0) {
            linenoiseAddCompletion(lc, commands[i].name);
        }
    }
    if(strncmp(buf, "q", 1) == 0) { linenoiseAddCompletion(lc, "exit"); }
}

char *hints(const char *buf, int *color,
            int *bold) { // use with the param lime print and in red <id>
    for(size_t i = 0; commands[i].name != NULL; i++) {
        if(strcmp(buf, commands[i].name) == 0) {
            if(commands[i].args_str) {
                *color = 32; // Green
                *bold = 0;
                return strdup(commands[i].args_str);
            }
        }
    }
    return NULL;
}

void free_hints(void *hint) { free(hint); }

int interactive_main_loop(void) {
    if(!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Input must be interactive\n");
        return 2;
    }

    printf("Welcome to LORD v%s\nType 'help' for commands\n", VERSION);

    const char *home = getenv("HOME");
    char history_path[512] = {0};
    if(home) {
        snprintf(history_path, sizeof(history_path), "%s/.lord_history", home);
        linenoiseHistoryLoad(history_path);
    } else {
        fprintf(stderr,
                "Could not get HOME environment variable, history will not be saved\n");
    }
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);
    linenoiseSetFreeHintsCallback(free_hints);

    char *line = linenoise(">>> ");
    while(line) {
        if(strlen(line) == 0) {
            linenoiseFree(line);
            line = linenoise(">>> ");
            continue;
        }
        linenoiseHistoryAdd(line);
        char **args = parse_input(line);
        if(strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0 ||
           strcmp(args[0], "q") == 0) {
            free(args);
            linenoiseFree(line);
            break;
        }
        execute_command(args);
        linenoiseFree(line);

        free(args);
        line = linenoise(">>> ");
    }

    if(strlen(history_path)) linenoiseHistorySave(history_path);

    return 0;
}
