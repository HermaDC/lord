#ifndef CLI_H
#define CLI_H

#include "stdbool.h"
typedef struct {
    int update_time;

    bool help;
    bool interactive;
    bool version;
    bool verbose;
    bool save;
    bool update_on_load;

    char *file;
    char *command;
    char *script;
    char *to_json;

} CLIOptions;

CLIOptions parse_args(int argc, char *argv[]);

void print_help();

#endif
