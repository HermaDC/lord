#include "interactive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "linenoise-lib/linenoise.h"
#include "types.h"
#include "utils.h"

#define TOKEN_CHUNK 64

// TODO add enum for for command exit status
// TODO add script option
typedef struct {
    char *msg;
    int invalid_arg_index;
    CmdErrorCode code;
    char *arg_value;
} CommandError;

typedef CommandError (*command_func)(char **args);

typedef struct {
    const char *name;
    command_func func;
    int min_args;
    const char *args_str;
    const char *usage;
    const char *desc;
} Command;

typedef struct {
    char *name;
    char *value;
} Variable;

typedef struct {
    Variable var_arr[100]; // TODO make this dynamic
    size_t var_count;
    size_t buf;
} VariableContext;

VariableContext var_context;

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
            if(!tokens) { return NULL; }
        }

        tokens[count++] = token;
        token = strtok(NULL, " ");
    }

    tokens[count] = NULL;
    return tokens;
}

// expands variables
static char **expand_args(char **args) {
    if(!args) return NULL;
    for(size_t i = 0; args[i] != NULL; i++) {
        if(args[i][0] == '$') {
            char *var_name = args[i] + 1;
            char *var_value = NULL;
            for(size_t j = 0; j < var_context.var_count; j++) {
                if(strcmp(var_context.var_arr[j].name, var_name) == 0) {
                    var_value = var_context.var_arr[j].value;
                    break;
                }
            }
            if(var_value) { args[i] = var_value; }
        }
    }
    return args;
}

static inline int count_args(char **args) {
    int i = 0;
    while(args[i])
        i++;
    return i;
}

CommandError command_show_help(char **args);
CommandError command_list_systems(char **args);
CommandError command_print_system(char **args);
CommandError command_save_system(char **args);
CommandError command_update_system(char **args);
CommandError command_clear_screen(char **args);
CommandError command_set_var(char **args);
CommandError command_print(char **args);

static const Command commands[] = {
    {"help", command_show_help, 0, NULL, "help", "Show this help"},
    {"list", command_list_systems, 0, NULL, "list", "List all systems"},
    {"print", command_print_system, 1, " <id>", "print <id>", "Print a system"},
    {"printf", command_print, 1, " <args...>", "print <args...>",
     "Print arguments with variable expansion"},
    {"save", command_save_system, 2, " <id> <name>", "save <id> <name>", "Save a system"},
    {"update", command_update_system, 1, " <id>", "update <id>", "Update a system"},
    {"clear", command_clear_screen, 0, NULL, "clear", "Clear the screen"},
    {"set", command_set_var, 2, " <var> <value>", "set <var> <value>",
     "Set a variable (not implemented)"},
    {NULL, NULL, 0, NULL, NULL, NULL}};

static CommandError execute_command(char **args) {
    if(!args || !args[0])
        return (CommandError){
            .code = CMD_ERR_INVALID_ARG, .msg = NULL, .invalid_arg_index = -1};

    int argc = count_args(args) - 1;

    for(size_t i = 0; commands[i].name != NULL; i++) {
        if(strcmp(args[0], commands[i].name) == 0) {

            if(argc < commands[i].min_args) {
                return (CommandError){
                    .code = CMD_ERR_TOO_FEW_ARGS, .msg = NULL, .invalid_arg_index = argc};
            }

            return commands[i].func(args);
        }
    }
    return (CommandError){
        .code = CMD_ERR_UNKNOWN_CMD, .msg = NULL, .invalid_arg_index = -1};
}

void show_error(CommandError err, char **args) {
    if(!args) return;
    CmdErrorCode err_code = err.code;
    // int arg_error = err.invalid_arg_index;
    // int pos_err=0;
    fprintf(stderr, RED "Error: " RESET "%s\n", cmderror_to_str(err_code));
    if(err.msg) fprintf(stderr, "%s\n", err.msg);
}

CommandError command_show_help(char **args) {
    (void) args;

    printf("Available commands:\n");

    for(size_t i = 0; commands[i].name != NULL; i++) {
        printf("  %-15s - %s\n", commands[i].usage, commands[i].desc);
    }
    printf("  %-15s - %s\n", "exit", "Exits the interactive session");

    return (CommandError){
        .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
}

CommandError command_list_systems(char **args) {
    (void) args;
    if(app_context.count == 0) {
        printf("No systems loaded\n");
        return (CommandError){
            .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
    }
    for(size_t i = 0; i < app_context.count; i++) {
        printf("System %zu: count %d\n", i, app_context.systems[i].count);
    }
    return (CommandError){
        .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
}

CommandError command_print_system(char **args) {
    char *endptr;
    int id = (int) strtol(args[1], &endptr, 10);
    if(*endptr != '\0') {
        return (CommandError){.code = CMD_ERR_INVALID_ARG,
                              .msg = "invalid system id",
                              .invalid_arg_index = 1,
                              .arg_value = args[1]};
    }
    if(id < 0 || id >= (int) app_context.count) {
        return (CommandError){.code = CMD_ERR_OUT_OF_BOUNDS,
                              .msg = "invalid system id",
                              .invalid_arg_index = 1,
                              .arg_value = args[1]};
    }
    print_tracks_with_switches(&app_context.systems[id], 0);
    printf("\n");
    return (CommandError){
        .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
}

CommandError command_save_system(char **args) {
    printf("Saving system: %s\n", args[1]);
    char *endptr;
    int id = (int) strtol(args[1], &endptr, 10);
    if(*endptr != '\0') {
        return (CommandError){.code = CMD_ERR_INVALID_ARG,
                              .msg = NULL,
                              .invalid_arg_index = 1,
                              .arg_value = args[1]};
    }
    if(id < 0 || id >= (int) app_context.count) {
        return (CommandError){.code = CMD_ERR_OUT_OF_BOUNDS,
                              .msg = "Invalid system id",
                              .invalid_arg_index = 1,
                              .arg_value = args[1]};
    }
    ErrorCode err = save_system_to_file(&app_context.systems[id], args[2]);
    if(err != ERR_OK) {
        fprintf(stderr, "Failed to save system: %s\n", args[1]);
        return (CommandError){
            .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
    }
    return (CommandError){
        .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
}

CommandError command_clear_screen(char **args) {
    (void) args;
    linenoiseClearScreen();
    return (CommandError){
        .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
}

CommandError command_update_system(char **args) {
    char *endptr;
    int id = (int) strtol(args[1], &endptr, 10);
    if(*endptr != '\0') {
        return (CommandError){.code = CMD_ERR_INVALID_ARG,
                              .msg = "Invalid system id",
                              .invalid_arg_index = 1,
                              .arg_value = args[1]};
    }
    if(id < 0 || id >= (int) app_context.count) {
        return (CommandError){.code = CMD_ERR_OUT_OF_BOUNDS,
                              .msg = "invalid system id",
                              .invalid_arg_index = 1,
                              .arg_value = args[1]};
    }
    update_system_status(&app_context.systems[id], 0);
    return (CommandError){
        .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
}

CommandError command_set_var(char **args) {
    if(count_args(args) < 2) {
        return (CommandError){.code = CMD_ERR_TOO_FEW_ARGS,
                              .msg = NULL,
                              .invalid_arg_index = count_args(args)};
    }
    for(size_t i = 0; i < var_context.var_count; i++) {
        if(strcmp(var_context.var_arr[i].name, args[1]) == 0) { // falla?
            free(var_context.var_arr[i].value);
            var_context.var_arr[i].value = strdup(args[2]);
            return (CommandError){.code = CMD_ERR_OK,
                                  .msg = NULL,
                                  .invalid_arg_index = -1,
                                  .arg_value = NULL};
        }
    }
    if(var_context.var_count < 100) {
        size_t index = var_context.var_count++;
        var_context.var_arr[index].name = strdup(args[1]);
        var_context.var_arr[index].value = strdup(args[2]);
        return (CommandError){
            .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};

    } else {
        return (CommandError){.code = CMD_ERR_OOM,
                              .msg = "Too much variables defined",
                              .invalid_arg_index = -1,
                              .arg_value = NULL};
    }
}

CommandError command_print(char **args) {
    if(count_args(args) < 1) {
        return (CommandError){.code = CMD_ERR_TOO_FEW_ARGS,
                              .msg = NULL,
                              .invalid_arg_index = count_args(args)};
    }
    for(size_t i = 1; args[i] != NULL; i++) {
        if(args[i][0] == '$') {
            // TODO variable expansion
            char *var_name = args[i] + 1;
            char *var_value = NULL;
            for(size_t j = 0; j < var_context.var_count; j++) {
                if(strcmp(var_context.var_arr[j].name, var_name) == 0) {
                    var_value = var_context.var_arr[j].value;
                    break;
                }
            }
            if(var_value) {
                printf("%s ", var_value);
            } else {
                printf("%s ", args[i]);
            }
        } else
            printf("%s ", args[i]);
    }
    printf("\n");
    return (CommandError){
        .code = CMD_ERR_OK, .msg = NULL, .invalid_arg_index = -1, .arg_value = NULL};
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

// returns 0 if ok, 1 if error, 2 if not TTY
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
        char *original_input = strdup(line);
        char **args = parse_input(line);
        args = expand_args(args);
        if(strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0 ||
           strcmp(args[0], "q") == 0) {
            free(args);
            free(original_input);
            linenoiseFree(line);
            break;
        }
        CommandError err = execute_command(args);
        if(err.code != CMD_ERR_OK) {
            // TODO use a pretty syntax highlighter
            // TODO add more info about the error like the argument index or a message
            // fprintf(stderr, "Error: Command execution failed with code %d\n", err);
            fprintf(stderr, "An error occur while executing '%s'\n", original_input);
            show_error(err, args);
        }
        linenoiseFree(line);
        free(original_input);

        free(args);
        line = linenoise(">>> ");
    }

    if(strlen(history_path)) linenoiseHistorySave(history_path);

    return 0;
}
