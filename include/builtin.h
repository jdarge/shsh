#ifndef BUILTIN_H_SHSH
#define BUILTIN_H_SHSH

#include "history.h"

#define CD 0x10
#define DOUBLE_BANG 0x11
#define HISTORY 0x12

int get_internal_command(char **args);

void cd_home();

int cd(char **args);

char** double_bang(History* h);

#endif