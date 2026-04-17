#ifndef CLI_H
#define CLI_H

typedef struct {
    int help;
    int interactive;
    int version;
    int verbose;
    int save;
    int update_time;

    char *file;
    char *command;
    char *script;

} CLIOptions;

CLIOptions parse_args(int argc, char *argv[]);

void print_help();

#endif
