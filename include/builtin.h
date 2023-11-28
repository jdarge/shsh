#ifndef SHSH_BUILTIN_H
#define SHSH_BUILTIN_H

#include "history.h"

#define CD 0x10
#define DOUBLE_BANG 0x11
#define HISTORY 0x12

int get_internal_command(char **args);

void cd_home(void);

int cd(char **args);

char **double_bang(History *h);

#endif
