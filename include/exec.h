#ifndef SHSH_EXEC_H
#define SHSH_EXEC_H

#include "history.h"

int shsh_execute(char **args, History *h);

int execute_single_command(char **args, History *h);

int execute_output_redirection(char **args, int redirect_index, History *h);

int execute_piped_commands(char **args, int pipe_index, History *h);

int exec_internal_command(char **args, int command, History *h);

void display_command_history(History *h);

#endif
