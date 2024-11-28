#include "history.h"

#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

History *history_init_block(void) {

    History *h = (History *) malloc(sizeof(History));
    h->list = (char **) malloc(sizeof(char) * HISTORY_SIZE);
    h->index = 0; // TODO SUS
    h->size = 1;

    return h;
}

void history_append(char *string, History *h) {

    int idx = h->index;
    char **t = h->list;

    if (idx >= (HISTORY_SIZE * h->size)) {
        ++h->size;
        char **tmp = realloc(h->list, HISTORY_SIZE * (h->size) * sizeof(char **));
        if (!tmp) {
            fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
            exit(EXIT_FAILURE);
        }
        h->list = tmp;
    } else if (idx > 0 && // this isn't the first entry, and we can safely check previous.
               strcmp(t[idx - 1], string) == 0 // check if the previous command == string
            ) {
        free(string);
    } else {
        t[idx] = string;
        h->index++;
    }
}
