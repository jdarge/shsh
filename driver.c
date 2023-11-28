#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "dtrie.h"
#include "env.h"
#include "exec.h"
#include "history.h"
#include "input.h"
#include "parse.h"
#include "trie.h"

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
    env = (ENV *) malloc(sizeof(ENV));
    env->path = (PATH *) malloc(sizeof(PATH));
    env->path->full = (char *) malloc(sizeof(char) * 256);

    strcpy(env->path->full, "/usr/local/bin:/usr/bin");
    parse_env_path(env);

    env->path->dt = dtrie_init();
    for(unsigned i = 0; i < env->path->parsed_count; i++)
        dtrie_insert_directory(env->path->dt, env->path->parsed[i]);

    /*
    TODO: obviously needs to be reworked 
    set up history block
    */
    History *h = history_init_block();

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

    // Free History
    for (int i = 0; i < h->history_idx; i++) {
        free(h->history_list[i]);
    }
    free(h->history_list);
    free(h);

    // Free ENV PATH DT
    trie_free(env->path->dt->trie->root);

    for (int i = 0; i < env->path->dt->trie->matchesSize * TRIE_MATCHES_SIZE; i++) {
        free(env->path->dt->trie->matches[i]);
    }
    free(env->path->dt->trie->matches);
    free(env->path->dt->trie->prefix);
    free(env->path->dt->trie);

    for (int i = 0; i < env->path->dt->dir_count; i++) {
        free(env->path->dt->directory[i]);
    }
    free(env->path->dt->directory);
    free(env->path->dt);

    // Free ENV PATH
    for(unsigned int i = 0; i < env->path->parsed_count; i++) {
        free(env->path->parsed[i]);
    }
    free(env->path->parsed);
    free(env->path->full);

    // Free ENV
    free(env->path);
    free(env);

    return 0;
}

// tab completion
// &&
//set up environment variables
