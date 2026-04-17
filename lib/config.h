#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include <stdbool.h>

struct Config {
    size_t MAX_STACK_AMOUNT;
    bool VERBOSE;
};

extern struct Config CONFIG;

#ifdef DEBUG
    #define SEED 1234
#else
    #define SEED time(NULL)
#endif

#ifndef LOG_PATH
    #define LOG_PATH "./gestion_trenes.log"
#endif

#ifndef MAX_STACK_SIZE
#define MAX_STACK_SIZE 32
#endif

#ifndef VERSION
#define VERSION "UNKNOWN"
#endif

#ifndef REPO_URL
#define REPO_URL "https://github.com/HermaDC/lord"
#endif

#endif // CONFIG_H
