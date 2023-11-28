#include "history.h"

#include <stdlib.h>
#include <string.h>

History *history_init_block(void) {
    
    History *h = (History *) malloc(sizeof(History));
    h->history_list = (char **) malloc(sizeof(char) * HISTORY_SIZE);
    h->history_idx = 0; // TODO SUS

    return h;
}

void history_append(char *string, History *h) {

    int idx = h->history_idx;
    char** t = h->history_list;

    if(idx >= HISTORY_SIZE) {
        free(string);
    } 

    else if(
        idx > 0 && // this isnt the first entry and we can safely check previous
        strcmp(t[idx - 1], string) == 0 // check if the previous command == string
    ) {
        free(string);
    } 

    else {
        t[idx] = (char*) malloc(sizeof(char) * (strlen(string) + 1));
        t[idx] = string;
        h->history_idx++;
    }
}
