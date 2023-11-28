#include "history.h"

#include <stdlib.h>

History *history_init_block(void) {
    History *h = (History *) malloc(sizeof(History));
    h->history_list = (char **) malloc(sizeof(char) * HISTORY_SIZE);
    h->history_idx = 0; // TODO SUS

    return h;
}

void history_append(char *string, History *h) {
    if (h->history_idx < HISTORY_SIZE) {
        h->history_list[h->history_idx++] = string;
        // h->history_idx++;
    }
}
