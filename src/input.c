#include "input.h"

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

char *read_line(char *b, int p, ENV *env, History *h) {

    int buffsize = 1024;
    int position = 0;
    char *buffer;
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
        printf("> %s", buffer);

    } else {
        buffer = malloc(sizeof(char) * buffsize);
    }

    if (!buffer) {
        fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
        exit(EXIT_FAILURE);
    }

    pthread_create(&input_thread, NULL, input_thread_function, &c);

    while (1) {

        pthread_join(input_thread, NULL);

        if (ctrl_c_pressed) {
            pthread_cancel(input_thread);
            ctrl_c_pressed = 0;

            for (int i = 0; i < position; i++) {
                printf("\b \b");
            }

            free(buffer);
            return NULL;
        }

        if (c == '\t') { // TODO!!!! need to use finish tab_comp and use char* comp
            char *completion = tab_completion(buffer, position, env);
            if (!completion) {
                printf("\n");
                return read_line(buffer, position, env, h);
            }
        }

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            printf("\n");
            return buffer;
        } else if (c == 27 && get_char() == 91) {
            int arrow_key = get_char();
            if (arrow_key == UP_ARROW) {
                if (local_history_idx - 1 >= 0) {
                    for (int i = 0; i < position; i++) {
                        printf("\b \b");
                    }

                    strcpy(buffer, h->history_list[--local_history_idx]);
                    position = (int) strlen(buffer);
                    printf("\r> %s", buffer);
                }
            } else if (arrow_key == DOWN_ARROW) {
                if (local_history_idx + 1 <= h->history_idx - 1) {
                    for (int i = 0; i < position; i++) {
                        printf("\b \b");
                    }

                    strcpy(buffer, h->history_list[++local_history_idx]);
                    position = (int) strlen(buffer);
                    printf("\r> %s", buffer);
                }
            } else if (arrow_key == RIGHT_ARROW) {//TODO
                if (position + 1 <= (int) strlen(buffer)) {
                    printf("\033[1C");
                    position++;
                }
            } else if (arrow_key == LEFT_ARROW) {//TODO
                if (position - 1 >= 0) {
                    printf("\033[1D");
                    position--;
                }
            }
        } else if (c == BACKSPACE) {
            if (position > 0) {
                position--;
                buffer[position] = '\0';
                printf("\r> %s ", buffer);
                printf("\b \b");
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

        pthread_create(&input_thread, NULL, input_thread_function, &c);
    }
}

int get_char(void) {
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

void ctrlC_handler(int signum) {

    (void) signum;

    ctrl_c_pressed = 1;
    printf("^C\n");
    fflush(stdout);

    pthread_cancel(input_thread);
}

void *input_thread_function(void *arg) {
    int *result = (int *) arg;
    *result = get_char();
    return NULL;
}

char *tab_completion(char *partial_input, int pos, ENV *env) {

    /*

        RETURN:
            NO HIT: 
                return NULL
            MULTI HIT:
                print all available
                return NULL
            SINGLE HIT: 
                update 'word' right before pos

        POSSIBLE DIRS:
            .
            PATH (
                /usr/local/bin
                /usr/bin
                /bin
            )            

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
    // printf("\n:%s:", partial_input);
    (void) env;
    (void) pos;

    DIR *dir;
    struct dirent *ent;
    char *completion = NULL;

    if ((dir = opendir(".")) == NULL) {
        perror("opendir");
        return partial_input;
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

    return NULL;
}
