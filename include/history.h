#ifndef SHSH_HISTORY_H
#define SHSH_HISTORY_H

#define HISTORY_SIZE 100

typedef struct History {
    char **history_list;
    int history_idx;
} History;

History *history_init_block(void);

void history_append(char *string, History *h);

/*

store history
load history

*/

#endif
