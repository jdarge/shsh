#ifndef SHSH_TRIE_H
#define SHSH_TRIE_H

#define CHARACTER_SET_SIZE 128
#define TRIE_PREFIX_SIZE 256
#define TRIE_MATCHES_SIZE 256

typedef struct TrieNode {
    struct TrieNode *children[CHARACTER_SET_SIZE];
    int is_end;
} TrieNode;

typedef struct Trie {
    struct TrieNode *root;

    char *prefix;
    int prefixSize;

    char **matches;
    int matchesCount;
    int matchesSize;
} Trie;

Trie *trie_init(void);

void trie_match_set_null(Trie* t, int l, int u);

TrieNode *trie_node_create(void);

void trie_insert(TrieNode *root, char *key);

void trie_search(Trie *t, char *key);

void trie_search_helper(TrieNode *current, Trie *t, int level);

void trie_print_words(TrieNode *root, char *prefix, int level);

void trie_free(TrieNode *root);

#endif
