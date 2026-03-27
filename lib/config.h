#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

struct Config {
    size_t MAX_STACK_AMOUNT;
};

extern const struct Config CONFIG;

#endif

#ifndef MAX_STACK_SIZE
#define MAX_STACK_SIZE 32
#endif

#ifndef VERSION
#define VERSION "UNKNOWN"
#endif

#ifndef REPO_URL
#define REPO_URL "https://github.com"
#endif
