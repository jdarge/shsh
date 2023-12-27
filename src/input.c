#include "input.h"

#include "dtrie.h"
#include "defs.h"
#include "env.h"
#include "history.h"

#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

volatile int ctrl_c_pressed = 0;

pthread_t input_thread;

char* read_line (char* b, int p, ENV* env, History* h) {

    int buffsize = 1024;
    int position = 0;
    char* buffer;
    int c;
    int local_history_idx = h->history_idx;

    if (b) {

        buffer = b;
        position = p;

        printf("\n");
        if (system("pwd")) {
            free(buffer);
            return NULL;
        }

        if (b[0] && p != 0) {
            printf("> %s", buffer);
        } else {
            printf("> ");
        }

    } else {
        buffer = malloc(sizeof(char) * buffsize);
        if (!buffer) {
            fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
            exit(EXIT_FAILURE);
        }
    }

    pthread_create(&input_thread, NULL, input_thread_function, &c);

    while (1) {

        pthread_join(input_thread, NULL);

        if (ctrl_c_pressed) {
            //pthread_cancel(input_thread);
            ctrl_c_pressed = 0;

            erase_buffer(position);

            free(buffer);
            return NULL;
        }

        if (c == '\t') { // TODO!!!! need to use finish tab_comp and use char* comp

            char* completion = tab_completion(buffer, position, env);

            if (!completion) {
                printf("\n");
                return read_line(buffer, position, env, h);
            }

            // TODO: replace partial with returned word and update pos

        }

        if (c == EOF || c == '\n') {
            if (position == 0) {
                goto next;
            }
            buffer[position] = '\0';
            printf("\n");
            return buffer;

        } else if (c == 27) {
            char o = get_char();
            if (o == 91) {
                int arrow_key = get_char();
                switch (arrow_key) {
                    case UP_ARROW:
                        handle_up_arrow(h->history_list, buffer, &local_history_idx, &position);
                        break;
                    case DOWN_ARROW:
                        handle_down_arrow(h->history_list, buffer, &local_history_idx, &position, h->history_idx);
                        break;
                    case RIGHT_ARROW:
                        handle_right_arrow(&position, (int) strlen(buffer));
                        break;
                    case LEFT_ARROW:
                        handle_left_arrow(&position);
                        break;
                }
            }

        } else if (c == BACKSPACE) {
            if (position > 0) {
                position--;
                buffer[position] = '\0';
                printf("\r> %s ", buffer);
                erase_buffer(1);
            }

        } else {
            if (!ctrl_c_pressed) {
                buffer[position] = (char) c;
                position++;
                buffer[position] = '\0';
                printf("\r> %s", buffer);
            }
        }

        if (position >= buffsize - 1) {
            buffsize += 1024;
            buffer = realloc(buffer, buffsize);

            if (!buffer) {
                fprintf(stderr, "shsh: Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        next:
        pthread_create(&input_thread, NULL, input_thread_function, &c);
    }
}

void erase_buffer (int count) {

    for (int i = 0; i < count; i++) {
        printf("\b \b");
    }
}

int get_char (void) {

    int ch;
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void handle_up_arrow (char** list, char* buffer, int* local_history_idx, int* position) {

    if (*local_history_idx - 1 >= 0) {
        erase_buffer(*position);
        strcpy(buffer, list[--(*local_history_idx)]);
        *position = (int) strlen(buffer);
        printf("\r> %s", buffer);
    }
}

void handle_down_arrow (char** list, char* buffer, int* local_history_idx, int* position, int limit) {

    if (*local_history_idx + 1 <= limit - 1) {
        erase_buffer(*position);
        strcpy(buffer, list[++(*local_history_idx)]);
        *position = (int) strlen(buffer);
        printf("\r> %s", buffer);
    }
}

void handle_right_arrow (int* position, int buffer_length) {

    if (*position + 1 <= buffer_length) {
        printf("\033[1C");
        (*position)++;
    }
}

void handle_left_arrow (int* position) {

    if (*position - 1 >= 0) {
        printf("\033[1D");
        (*position)--;
    }
}

void ctrlC_handler (int signum) {

    (void) signum;

    ctrl_c_pressed = 1;
    printf("^C\n");
    fflush(stdout);

    pthread_cancel(input_thread);
}

void ctrlL_handler (int signum) {

    (void) signum;

    printf("\033[2J\033[H");
    fflush(stdout);
}

void* input_thread_function (void* arg) {

    int* result = (int*) arg;
    *result = get_char();
    return NULL;
}

char* tab_completion (char* partial_input, int pos, ENV* env) {

    /*
        RETURN:
            NO HIT: 
                return NULL
            MULTI HIT:
                print all available
                return NULL
            SINGLE HIT: 
                update 'word' right before pos
                return corrected string at pos
    */
    /*
        POSSIBLE DIRS:
            .
            PATH (
                /usr/local/bin
                /usr/bin
                /bin
            )            
    */
    /*
        1. tokenize partial_input until we get to pos
            i.e.
            cat mai\t
            mai
        2. run main against PATH and CWD
        3. if theres matches put them inside char** hits
        4. print values in hits
            i.e.

            > ls\t
            ls                  lsipc               lspci 

            > ls

        a. perhaps use $column to make this print
        5. on multiple hits NOTHING should be done
        6. if theres only one hit, complete and return
    */
    /*
        ADDITIONAL NOTES:
            > ls\t
            ls                  lsipc           lspci

            > ls \t
            main.c     README.md  run.sh     shsh 
        
        notice how context changes based on what's available

        perhaps in cases where its 
        cat main.c | grep \t 
        first tokenize string
            cat,main.c,|,grep,\t
        figure where pos is (\t)
        discount all strings prior to main command like... 
        grep,\t
        if grep is in /.../bin
        show dir context
        if it's the first command
        show bin context
    */

    (void) pos;

    // TODO: this should only be done if there's no command called previous
    DirecTrie* d = env->path->dt;
    dtrie_search(d, partial_input);
    if (d->trie->matchesCount == 1) {
        return d->trie->matches[0];
    } else {// 0 || >1
        if (d->trie->matchesCount != 0) {
            printf("\n");
            for (int i = 0; i < d->trie->matchesCount; i++) {
                printf("%-25s", strrchr(d->trie->matches[i], '/') + 1);
                if ((i + 1) % 3 == 0 || i == d->trie->matchesCount - 1) {
                    printf("\n");
                }
            }
        }
        
        return NULL;
    }

    // TODO: if command called previous check only CWD
    //  (dont bother with trie & include subdir)
}

/*  TAB COMPLETION

    DIR *dir;
    struct dirent *ent;
    char *completion = NULL;

    if ((dir = opendir(".")) == NULL) {
        perror("opendir");
        return NULL;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, partial_input, strlen(partial_input)) == 0) {
            if (!completion) {
                completion = strdup(ent->d_name);
            } else {
                size_t i;
                for (i = 0; i < strlen(completion) && i < strlen(ent->d_name); i++) {
                    if (completion[i] != ent->d_name[i]) {
                        break;
                    }
                }

                completion[i] = '\0';
                break;
            }
        }
    }
    closedir(dir);

    if (NULL) {
    }
*/
