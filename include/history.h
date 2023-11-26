#ifndef HISTORY_H_SHSH
#define HISTORY_H_SHSH

#define HISTORY_SIZE 100

typedef struct History {
    char** history_list;
    int history_idx;
} History;

History* history_init_block();

/*

store history
load history

*/

#endif