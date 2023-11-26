#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "env.h"
#include "defs.h"
#include "parse.h"
#include "input.h"
#include "history.h"
#include "builtin.h"
#include "exec.h"

ENV *env; // TODO

int main(void) {
    char *line;
    char **args;
    int status;

    /*
    TODO:
    this functions as a temporary setup for waht will become environment variables
    i.e. $PATH

    char *new_path = malloc(strlen(current_path) + strlen(additional_path) + 2);
    sprintf(new_path, "%s:%s", current_path, additional_path);
    */
    env = (ENV*) malloc(sizeof(ENV));
    env->path = (PATH*) malloc(sizeof(PATH));
    env->path->full = (char*) malloc(sizeof(char) * 256);
    strcpy(env->path->full, "/usr/local/bin:/usr/bin:/bin");
    parse_env_path(env);

    /*
    TODO: obviously needs to be reworked 
    set up history block
    */
    History* h = history_init_block();

    do {

        signal(SIGINT, ctrlC_handler);

        if (system("pwd"))
            return 1;
        printf("> ");

        line = read_line(NULL, 0, env, h);
        args = split_line(line);
        status = shsh_execute(args, h);

        if (line)
            free(line);
        if (args)
            free(args);
        if (status)
            printf("\n");
    } while (status);

    for (int i = 0; i < h->history_idx; i++) {
        free(h->history_list[i]);
    } free(h);

    return 0;
}

// tab completion
// &&
//set up environment variables
