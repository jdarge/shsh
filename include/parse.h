#ifndef SHSH_PARSE_H
#define SHSH_PARSE_H

#include "defs.h"
#include "env.h"

#define TK_BUFF_SIZE 64
#define TOK_DELIM DELIMITER_RESET

char**
split_line (char* line);

char*
concatenate_strings (char** args);

void
parse_env_path (ENV* env);

#endif
