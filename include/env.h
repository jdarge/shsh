#ifndef SHSH_ENV_H
#define SHSH_ENV_H

#include "dtrie.h"

typedef struct PATH {
    DTrie *dt; // todo, find a more appropriate spot
    char *full;
    char **parsed;
    unsigned int parsed_count;
} PATH;

typedef struct ENV {
    PATH *path;
} ENV;

#endif
