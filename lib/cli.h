#ifndef CLI_H
#define CLI_H

typedef struct {
    int help;
    int interactive;
    int version;
    int verbose;

    char *file;
    char *command;
    int update_time;

} CLIOptions;

CLIOptions parse_args(int argc, char *argv[]);

void print_help();

#endif
