#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "cli.h"
#include "config.h"

CLIOptions parse_args(int argc, char *argv[]) {
    CLIOptions opts = {0};

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"file", required_argument, 0, 'f'},
        {"interactive", no_argument, 0, 'i'},
        {"version", no_argument, 0, 'v'},
        {"command", required_argument, 0, 'c'},
        {"update", required_argument, 0, 'u'},
        {"verbose", no_argument, 0, 0},
        {0, 0, 0, 0}
    };

    int opt;
    int long_index = 0;

    while ((opt = getopt_long(argc, argv, "hf:ivc:u:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                opts.help = 1;
                break;
            case 'f':
                opts.file = optarg;
                break;
            case 'i':
                opts.interactive = 1;
                break;
            case 'v':
                opts.version = 1;
                break;
            case 'c':
                opts.command = optarg;
                break;
            case 'u':
                opts.update_time = atoi(optarg);
                break;
            case 0: // long options without short version
                if (long_options[long_index].name &&
                    strcmp(long_options[long_index].name, "verbose") == 0) {
                    opts.verbose = 1;
                }
                break;
            default:
                print_help();
                exit(1);
        }
    }

    return opts;
}

void print_help() {
    printf("Uso:\n");
    printf("  -h, --help              Show help\n");
    printf("  -f, --file PATH         Load config file\n");
    printf("  -i, --interactive       Runs in interactive mode\n");
    printf("  -v, --version           Shows version\n");
    printf("  -c, --command CMD       Runs CMD\n");
    printf("  -u, --update MS         Time in miliseconds between updates\n");
    printf("      --verbose           Be more verbose\n");
    printf("Report bugs at %s/issues\n", REPO_URL);
}
