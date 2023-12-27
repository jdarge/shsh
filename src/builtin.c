#include "builtin.h"

#include "history.h"
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

int get_internal_command (char** args) {

    if (strcmp(args[0], "cd") == 0) {
        return CD;
    } else if (strcmp(args[0], "!!") == 0) {
        return DOUBLE_BANG;
    } else if (strcmp(args[0], "history") == 0) {
        return HISTORY;
    }

    return 0;
}

void cd_home (void) {

    const char* home = getenv("HOME");
    if (!home) {
        perror("The HOME environment variable is not set.");
    } else {
        if (chdir(home) != 0) {
            perror("chdir");
        }
    }
}

int cd (char** args) {

    if (args[1] == NULL) {
        cd_home();
        return CD;
    }

    wordexp_t p;
    if (wordexp(args[1], &p, WRDE_UNDEF) != 0) {
        perror("wordexp");
        return 1;
    }

    if (p.we_wordc != 1) {
        perror("Invalid directory specification.");
    } else {
        if (strcmp(p.we_wordv[0], "~") == 0) {
            cd_home();
        } else {
            if (chdir(p.we_wordv[0]) != 0) {
                perror("chdir");
            }
        }
    }

    wordfree(&p);

    return CD;
}

char** double_bang (History* h) {

    if (h->history_idx != 0) {
        return split_line(h->history_list[h->history_idx - 1]);
    }

    return NULL;
}
