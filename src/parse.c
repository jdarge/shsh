#include "parse.h"

#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **split_line(char *line) {

    if (!line)
        return NULL;

    int buffsize = TK_BUFF_SIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buffsize) {
            buffsize += TK_BUFF_SIZE;
            tokens = realloc(tokens, buffsize * sizeof(char *));

            if (!tokens) {
                fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }

    tokens[position] = NULL;

    return tokens;
}

char *concatenate_strings(char **args) {
    size_t totalLength = 0;
    for (int i = 0; args[i] != NULL; i++) {
        totalLength += strlen(args[i]) + 1;
    }

    char *result = (char *) malloc(totalLength + 1);

    if (result == NULL) {
        return NULL;
    }

    result[0] = '\0';
    for (int i = 0; args[i] != NULL; i++) {
        strcat(result, args[i]);
        if (args[i + 1] != NULL)
            strcat(result, " ");
    }

    return result;
}

void parse_env_path(ENV *env) {
    // WILL allocate memory for parsed paths
    // WILL update PATH parsed_count

    char *full = env->path->full;

    char *lp = (char *) malloc(sizeof(char) * strlen(env->path->full) + 1);
    strcpy(lp, full);

    int index = 0;
    for (int i = 0; i < (int) strlen(lp); i++)
        if (lp[i] == ':')
            index++;

    char **parsed_paths = (char **) malloc(sizeof(char *) * index);

    int i = 0;
    char *token = strtok(lp, ":");
    while (token != NULL) {
        parsed_paths[i] = (char *) malloc(strlen(token) + 1);
        strcpy(parsed_paths[i], token);
        i++;
        token = strtok(NULL, ":");
    }

    env->path->parsed = parsed_paths;
    env->path->parsed_count = index + 1;

    free(lp);
}
