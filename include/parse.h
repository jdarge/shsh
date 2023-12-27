#ifndef SHSH_PARSE_H
#define SHSH_PARSE_H

#include "env.h"

#define TK_BUFF_SIZE 64
#define TOK_DELIM " \t\r\n"

char** split_line (char* line);

char* concatenate_strings (char** args);

void parse_env_path (ENV* env);

#endif
