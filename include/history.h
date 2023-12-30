#ifndef SHSH_HISTORY_H
#define SHSH_HISTORY_H

#define HISTORY_SIZE 100

typedef struct History
{
    char** list;
    int index;
    int size;
} History;

History*
history_init_block (void);

void
history_append (char* string, History* h);

/*

store history
load history

*/

#endif
