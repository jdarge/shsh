#include "dtrie.h"
#include "trie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

DTrie*
dtrie_init (void)
{
    DTrie* d = (DTrie*) malloc(sizeof(DTrie));

    d->directory = NULL;
    d->dir_count = 0;

    d->trie = trie_init();

    return d;
}

void
dtrie_insert_directory (DTrie* d, char* dirPath)
{
    DIR* directory;
    struct dirent* entry;

    if ((directory = opendir(dirPath)) == NULL)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char filePath[PATH_MAX];
            snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

            trie_insert(d->trie->root, filePath);

            // if (entry->d_type == DT_DIR) {
            //     insertFilesInDirectory(d, filePath);
            // }
        }
    }

    closedir(directory);


    char** tmp = realloc(d->directory, (d->dir_count + 1) * sizeof(char*));
    
    if (!tmp)
    {
        // TODO
    }
    else
    {
        d->directory = tmp;
    }
    
    d->directory[d->dir_count] = (char*) malloc(strlen(dirPath) + 1);
    strcpy(d->directory[d->dir_count], dirPath);
    d->dir_count++;
}

void
dtrie_search (DTrie* d, char* key)
{
    char* path = (char*) calloc(TRIE_PREFIX_SIZE, sizeof(char));
    d->trie->matchesCount = 0;

    for (int i = 0; i < d->dir_count; i++)
    {

        strcpy(path, d->directory[i]);
        if (path[strlen(d->directory[i]) - 1] != '/')
        {
            strcat(path, "/");
        }

        strcat(path, key);
        path[strlen(path)] = '\0';

        trie_search(d->trie, path);
    }

    free(path);
}
