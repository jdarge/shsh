#ifndef SHSH_DTRIE_H
#define SHSH_DTRIE_H

#include "trie.h"

typedef struct DirecTrie {
    struct Trie *trie;
    char **directory;
    int dir_count;
} DirecTrie;

DirecTrie *dtrie_init(void);
void dtrie_insert_directory(DirecTrie *d, char *dirPath);
void dtrie_search(DirecTrie *d, char *key);

#endif