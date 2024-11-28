#include "parse.h"

#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **split_line(char *line) {

    if (!line) {
        return NULL;
    }

    int buffer_size = TK_BUFF_SIZE, position = 0;
    char **tokens = malloc(buffer_size * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buffer_size) {
            buffer_size += TK_BUFF_SIZE;

            char **tmp = realloc(tokens, buffer_size * sizeof(char *));
            if (!tmp) {
                fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
                exit(EXIT_FAILURE);
            }
            tokens = tmp;
        }
        token = strtok(NULL, TOK_DELIM);
    }

    tokens[position] = NULL;

    return tokens;
}

char *concatenate_strings(char **args) {

    size_t len = 0;
    for (int i = 0; args[i] != NULL; i++) {
        len += strlen(args[i]) + 1;
    }

    char *result = (char *) malloc(len + 1);

    if (result == NULL) {
        return NULL;
    }

    result[0] = '\0';
    for (int i = 0; args[i] != NULL; i++) {
        strcat(result, args[i]);
        if (args[i + 1] != NULL) {
            strcat(result, " ");
        }
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
    for (int i = 0; i < (int) strlen(lp); i++) {
        if (lp[i] == ':') {
            index++;
        }
    }

    char **parsed_paths = (char **) malloc(sizeof(char *) * index);

    int i = 0;
    char *token = strtok(lp, ":");
    while (token != NULL) {
        parsed_paths[i] = (char *) malloc(strlen(token) + 1);
        strcpy(parsed_paths[i], token);
        token = strtok(NULL, ":");
        i++;
    }

    env->path->parsed = parsed_paths;
    env->path->parsed_count = index + 1;

    free(lp);
}
