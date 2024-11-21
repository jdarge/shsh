#include "input.h"

#include "dtrie.h"
#include "defs.h"
#include "env.h"
#include "history.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

volatile int ctrl_c_pressed = 0;

pthread_t input_thread;

char*
read_line (char* b, int p, ENV* env, History* h)
{
    int buffsize = 1024;
    int position = 0;
    char* buffer;
    int c;
    int local_history_idx = h->index;

    if (b)
    {
        buffer = b;
        position = p;

        printf("\n");
        if (system("pwd"))
        {
            free(buffer);
            return NULL;
        }

        if (b[0] && p != 0)
        {
            printf("> %s", buffer);
        }
        else
        {
            printf("> ");
        }
    }
    else
    {
        buffer = calloc(0, sizeof(char) * buffsize);
        if (!buffer)
        {
            fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
            exit(EXIT_FAILURE);
        }
    }

    pthread_create(&input_thread, NULL, input_thread_function, &c);

    while (1)
    {
        pthread_join(input_thread, NULL);

        if (ctrl_c_pressed)
        {
            ctrl_c_pressed = 0;

            printf("\r> %s", buffer);
            printf("^C\n");

            free(buffer);
            return NULL;
        }

        if (c == '\t')
        {
            char* completion = tab_completion(buffer, position, env);

            if (completion) {
                char* last_part = strrchr(completion, '/');
                if (last_part) {
                    last_part++;
                } else {
                    last_part = completion;
                }

                int common_len = position;
                if (strncmp(buffer, last_part, common_len) == 0) {
                    char* remaining_part = last_part + common_len;

                    int remaining_len = strlen(remaining_part);
                    if (position < strlen(buffer)) {
                        memmove(
                            buffer + position + remaining_len, 
                            buffer + position, 
                            strlen(buffer) - position + 1
                        );
                    }

                    strncpy(buffer + position, remaining_part, remaining_len);

                    position += remaining_len;
                }
            }

            printf("\n");
            pthread_join(input_thread, NULL);
            return read_line(buffer, position, env, h);
        }

        if (c == EOF || c == '\n')
        {
            if (position == 0)
            {
                goto next;
            }

            printf("\n");
            return buffer;
        }
        else if (c == 27)
        {   // TODO! add ctrl + L/R Arrow handler to jump around whitespace
            char o = (char) get_char();
            if (o == 91)
            {
                int arrow_key = get_char();
                switch (arrow_key)
                {
                    case UP_ARROW:
                        handle_up_arrow(h->list, buffer, &local_history_idx, &position);
                        break;
                    case DOWN_ARROW:
                        handle_down_arrow(h->list, buffer, &local_history_idx, &position, h->index);
                        break;
                    case RIGHT_ARROW:
                        handle_right_arrow(&position, (int) strlen(buffer));
                        break;
                    case LEFT_ARROW:
                        handle_left_arrow(&position);
                        break;
                }
            }
        } else if (c == CTRL_L)
        {
            printf("\033[H\033[2J"); 
            printf("\n");
            fflush(stdout);
            system("pwd");
            printf("\r> %s", buffer);
        }
        else if (c == BACKSPACE)
        {
            if (position > 0)
            {
                position--;
                memmove(buffer + position, buffer + position + 1, strlen(buffer) - position);
                int len_after_cursor = (int) strlen(buffer) - position;

                printf("\r> %s", buffer);
                printf(" \b");
                for (int i = 0; i < len_after_cursor; i++)
                {
                    printf("\b");
                }
            }
        }
        // TODO! add HOME/END key handler
        else
        {   // Ran through all checks
            if (!ctrl_c_pressed)
            {
                if (position < (int) strlen(buffer))
                {
                    memmove(buffer + position + 1, buffer + position, strlen(buffer) - position + 1);
                }

                buffer[position] = (char) c;
                position++;
                printf("\r> %s", buffer);

                int len_after_cursor = (int) strlen(buffer) - position;
                for (int i = 0; i < len_after_cursor; i++)
                {
                    printf("\b");
                }
            }
        }

        if (position >= buffsize - 1)
        {
            buffsize += 1024;

            char* tmp = realloc(buffer, buffsize);
            if (!tmp)
            {
                fprintf(stderr, "shsh: Allocation error\n");
                exit(EXIT_FAILURE);
            }

            memset(tmp + position, '\0', buffsize - position);

            buffer = tmp;
        }

        next:
        pthread_create(&input_thread, NULL, input_thread_function, &c);
    }
}

void
erase_buffer (int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("\b \b");
    }
}

int
get_char (void)
{
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

void
handle_up_arrow (char** list, char* buffer, int* local_history_idx, int* position)
{
    if (*local_history_idx - 1 >= 0)
    {
        erase_buffer(*position);
        strcpy(buffer, list[--(*local_history_idx)]);
        *position = (int) strlen(buffer);
        printf("\r> %s", buffer);
    }
}

void
handle_down_arrow (char** list, char* buffer, int* local_history_idx, int* position, int limit)
{
    if (*local_history_idx + 1 <= limit - 1)
    {
        erase_buffer(*position);
        strcpy(buffer, list[++(*local_history_idx)]);
        *position = (int) strlen(buffer);
        printf("\r> %s", buffer);
    }
}

void
handle_right_arrow (int* position, int buffer_length)
{
    if (*position + 1 <= buffer_length)
    {
        printf("\033[1C");
        (*position)++;
    }
}

void
handle_left_arrow (int* position)
{
    if (*position - 1 >= 0)
    {
        printf("\033[1D");
        (*position)--;
    }
}

void
ctrlC_handler (int signum)
{
    (void) signum;

    ctrl_c_pressed = 1;    

    pthread_cancel(input_thread);
}

void*
input_thread_function (void* arg)
{
    int* result = (int*) arg;
    *result = get_char();
    return NULL;
}

char*
tab_completion (char* partial_input, int pos, ENV* env)
{

    // TODO! this should only be done if there's no command called previous

    DTrie* d = env->path->dt;
    dtrie_search(d, partial_input);
    if (d->trie->matchesCount == 1)
    {
        return d->trie->matches[0];
    }
    else if (d->trie->matchesCount != 0)
    {
        printf("\n");
        for (int i = 0; i < d->trie->matchesCount; i++)
        {
            printf("%-25s", strrchr(d->trie->matches[i], '/') + 1);
            if ((i + 1) % 3 == 0 && i != d->trie->matchesCount - 1)
            {
                printf("\n");
            }
        }
    }

    return NULL;
}
