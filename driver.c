#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "dtrie.h"
#include "env.h"
#include "exec.h"
#include "history.h"
#include "input.h"
#include "parse.h"
#include "trie.h"

ENV* env; // TODO

int
main (void)
{

    char* line;
    char** args;
    int status;
    char cwd[256];

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

    strcpy(env->path->full, "/usr/local/bin:/usr/bin");
    parse_env_path(env);

    env->path->dt = dtrie_init();
    for (unsigned i = 0; i < env->path->parsed_count; i++)
    {
        dtrie_insert_directory(env->path->dt, env->path->parsed[i]);
    }

    /*
    TODO: obviously needs to be reworked 
    set up history block
    */
    History* h = history_init_block();

    do
    {

        signal(SIGINT, ctrlC_handler);
        // signal(SIGWINCH, ...); // FIXME: SIGWINCH = resize handler

        if (getcwd(cwd, sizeof(cwd)) == 0) 
        {
            return 1;
        }
        
        printf("%s%s%s\n> ", RED, cwd, RESET);

        line = read_line(NULL, 0, env, h, 0);
        args = split_line(line);
        status = shsh_execute(args, h);

        if (line)
        {
            free(line);
        }
        if (args)
        {
            free(args);
        }
        if (status)
        {
            printf("\n");
        }
    } while (status);

    // Free History
    char** hl = h->list;
    for (int i = 0; i < h->index; i++)
    {
        free(hl[i]);
    }
    free(hl);
    free(h);

    // Free ENV PATH DT
    DTrie* dt = env->path->dt;
    Trie* t = dt->trie;
    trie_free(t->root);
    for (int i = 0; i < t->matchesSize * TRIE_MATCHES_SIZE; i++)
    {
        if (t->matches[i])
        {
            free(t->matches[i]);
        }
    }
    free(t->matches);
    free(t->prefix);
    free(t);

    for (int i = 0; i < dt->dir_count; i++)
    {
        free(dt->directory[i]);
    }
    free(dt->directory);
    free(dt);

    // Free ENV PATH
    PATH* p = env->path;
    for (unsigned int i = 0; i < p->parsed_count; i++)
    {
        free(p->parsed[i]);
    }
    free(p->parsed);
    free(p->full);
    free(p);

    // Free ENV
    free(env);

    return 0;
}

// tab completion
// &&
//set up environment variables
