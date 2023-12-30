#ifndef SHSH_DTRIE_H
#define SHSH_DTRIE_H

#include "trie.h"

typedef struct DTrie
{
    struct Trie* trie;
    char** directory;
    int dir_count;
} DTrie;

DTrie*
dtrie_init (void);

void
dtrie_insert_directory (DTrie* d, char* dirPath);

void
dtrie_search (DTrie* d, char* key);

#endif
