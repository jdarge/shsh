#include "trie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Trie*
trie_init (void)
{
    Trie* t = (Trie*) malloc(sizeof(Trie));

    t->prefix = (char*) malloc((TRIE_PREFIX_SIZE + 1) * sizeof(char));
    memset(t->prefix, '\0', (TRIE_PREFIX_SIZE + 1) * sizeof(char));

    t->prefixSize = TRIE_PREFIX_SIZE;

    t->matches = (char**) malloc(sizeof(char*) * TRIE_MATCHES_SIZE);
    t->matchesCount = 0;
    t->matchesSize = 1;

    t->root = trie_node_create();

    trie_match_set_null(t, 0, TRIE_MATCHES_SIZE);

    return t;
}

void
trie_match_set_null (Trie* t, int l, int u)
{
    for (int i = l; i < u; i++)
    {
        t->matches[i] = NULL;
    }
}

TrieNode*
trie_node_create (void)
{
    TrieNode* node = (TrieNode*) malloc(sizeof(TrieNode));

    if (node == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++)
    {
        node->children[i] = NULL;
    }

    node->is_end = 0;
    return node;
}

void
trie_insert (TrieNode* root, char* key)
{
    TrieNode* current = root;
    unsigned long len = strlen(key);

    for (unsigned long level = 0; level < len; level++)
    {
        int index = (unsigned char) key[level];
        if (!current->children[index])
        {
            current->children[index] = trie_node_create();
        }
        current = current->children[index];
    }

    current->is_end = 1;
}

void
trie_search (Trie* t, char* key)
{
    TrieNode* current = t->root;
    unsigned long len = strlen(key);
    unsigned long prefix_size = len;
    char* tmp;

    for (unsigned long level = 0; level < len; level++)
    {
        int index = (unsigned char) key[level];
        if (!current->children[index])
        {
            return;
        }

        if (prefix_size >= TRIE_PREFIX_SIZE)
        {
            tmp = realloc(t->prefix, (prefix_size + 1) * sizeof(char));
            if (!tmp)
            {
                // TODO
            }
            else
            {
                t->prefix = tmp;
            }
        }

        if (t->prefix == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        t->prefix[level] = key[level];
        t->prefix[level + 1] = '\0';

        current = current->children[index];
    }

    trie_search_helper(current, t, len);
}

void
trie_search_helper (TrieNode* current, Trie* t, unsigned long level)
{
    if (current->is_end)
    {
        if (t->matchesCount + 1 >= TRIE_MATCHES_SIZE * t->matchesSize)
        {
            char** tmp = realloc(t->matches, ++t->matchesSize * TRIE_MATCHES_SIZE * sizeof(char*));

            if (!tmp)
            {
                // TODO
            }
            else
            {
                t->matches = tmp;
            }

            trie_match_set_null(t, t->matchesCount, t->matchesSize * TRIE_MATCHES_SIZE);
        }

        if (t->matches == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        if (t->matches[t->matchesCount] != NULL)
        {
            free(t->matches[t->matchesCount]);
        }
        t->matches[t->matchesCount++] = strdup(t->prefix);
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++)
    {
        if (current->children[i])
        {
            t->prefix[level] = (char) i;
            trie_search_helper(current->children[i], t, level + 1);
            t->prefix[level] = '\0';
        }
    }
}

void
trie_print_words (TrieNode* root, char* prefix, unsigned long level)
{
    if (root->is_end)
    {
        prefix[level] = '\0';
        printf("%s\n", prefix);
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++)
    {
        if (root->children[i])
        {
            prefix[level] = (char) i;
            trie_print_words(root->children[i], prefix, level + 1);
        }
    }
}

void
trie_free (TrieNode* root)
{
    if (root == NULL)
    {
        return;
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++)
    {
        trie_free(root->children[i]);
    }

    free(root);
}
