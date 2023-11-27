#include "history.h"

#include <stdlib.h>

History *history_init_block(void) {
    History *h = (History *) malloc(sizeof(History));
    h->history_list = (char **) malloc(sizeof(char) * HISTORY_SIZE);
    h->history_idx = 0; // TODO SUS

    return h;
}
