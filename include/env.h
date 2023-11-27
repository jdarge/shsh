#ifndef ENV_H_SHSH
#define ENV_H_SHSH

typedef struct PATH {
    char *full;
    char **parsed;
    unsigned int parsed_count;
} PATH;

typedef struct ENV {
    PATH *path;
} ENV;

#endif