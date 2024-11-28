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
pthread_mutex_t ctrl_c_mutex = PTHREAD_MUTEX_INITIALIZER;

char*
read_line (char* buffer_input, int position_input, ENV* env, History* history, int flag)
{
    int buffsize = 1024;
    int position = 0;
    int local_history_idx = history->index;
    char cwd[256];
    char* buffer;
    int current_user_input;

    if (buffer_input)
    {
        buffer = buffer_input;
        position = position_input;
        buffsize = (int)(position/1024) * 1024;

        if (getcwd(cwd, sizeof(cwd)) == 0) {
            free(buffer);
            return NULL;
        }

        if (flag & SKIP_CWD) {
            print_buffer(buffer);
        } else if (buffer_input[0] && position_input != 0) {
            printf("\n%s%s%s\n> %s", RED, cwd, RESET, buffer);
        } else {
            printf("\n%s%s%s\n> ", RED, cwd, RESET);
        }
    }
    else
    {
        buffer = calloc(buffsize, sizeof(char));
        if (!buffer) {
            fprintf(stderr, "%sshsh: Allocation error%s\n", RED, RESET);
            exit(EXIT_FAILURE);
        }
    }

    pthread_create(&input_thread, NULL, input_thread_function, &current_user_input);

    while (1)
    {
        pthread_join(input_thread, NULL);

        if (ctrl_c_pressed)
        {
            ctrl_c_pressed = 0;

            print_buffer(buffer);
            printf("^C\n");

            free(buffer);
            return NULL;
        }

        if (current_user_input == '\t')
        {   // TODO! this need to be put into it's own function or simplified...
            char* completion = tab_completion(buffer, position, env);

            if (completion) {
                int completion_len = strlen(completion);

                if (completion_len > 0 && completion[completion_len - 1] == '/') {
                    completion[completion_len - 1] = '\0';
                    completion_len--;
                }

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
                        memmove(buffer + position + remaining_len, buffer + position, strlen(buffer) - position + 1);
                    }

                    strncpy(buffer + position, remaining_part, remaining_len);

                    position += remaining_len;

                    buffer[position] = ' ';
                    position++; 
                    buffer[position] = '\0'; 
                }

                pthread_join(input_thread, NULL);
                return read_line(buffer, position, env, history, SKIP_CWD);
            }
            else 
            {
                printf("\n");
                pthread_join(input_thread, NULL);
                return read_line(buffer, position, env, history, 0);
            }
        }

        if (current_user_input == EOF || current_user_input == '\n')
        {
            if (position == 0)
            {
                pthread_create(&input_thread, NULL, input_thread_function, &current_user_input);
                continue;
            }

            printf("\n");
            return buffer;
        }
        else if (current_user_input == ESCAPE)
        {   // TODO! add ctrl + L/R Arrow handler to jump around whitespace
            char current_user_input_buffer = (char) get_char();
            if (current_user_input_buffer == OPEN_BRACKET)
            {
                int key = get_char();
                
                if(key == UP_ARROW || key == DOWN_ARROW || key == RIGHT_ARROW || key == LEFT_ARROW) 
                {
                    handle_arrow(key, history, buffer, &local_history_idx, &position);
                }
            }
        } 
        else if (current_user_input == CTRL_L)
        {
            printf("\033[H\033[2J"); 
            printf("\n");
            fflush(stdout);
            system("pwd");
            print_buffer(buffer);
        }
        else if (current_user_input == BACKSPACE && position > 0)
        {
            position--;
            memmove(buffer + position, buffer + position + 1, strlen(buffer) - position);
            
            print_buffer(buffer);
            
            int len_after_cursor = (int) strlen(buffer) - position;
            printf(" \b");
            for (int i = 0; i < len_after_cursor; i++)
            {
                printf("\b");
            }
        }
        // TODO! add HOME/END key handler
        else if (!ctrl_c_pressed)
        {   // Ran through all checks
            if (position < (int) strlen(buffer))
            {
                memmove(buffer + position + 1, buffer + position, strlen(buffer) - position + 1);
            }

            buffer[position] = (char) current_user_input;
            position++;
            print_buffer(buffer);

            int len_after_cursor = (int) strlen(buffer) - position;
            for (int i = 0; i < len_after_cursor; i++)
            {
                printf("\b");
            }
        }

        if (position + 1 >= buffsize - 1)
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

        pthread_create(&input_thread, NULL, input_thread_function, &current_user_input);
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
handle_arrow (int arrow_key, History* history, char* buffer, int* local_history_idx, int* position)
{
    switch (arrow_key)
    {
        case UP_ARROW:
            handle_up_arrow(history->list, buffer, local_history_idx, position);
            break;
        case DOWN_ARROW:
            handle_down_arrow(history->list, buffer, local_history_idx, position, history->index);
            break;
        case RIGHT_ARROW:
            handle_right_arrow(position, (int) strlen(buffer));
            break;
        case LEFT_ARROW:
            handle_left_arrow(position);
            break;
    }
}

void
handle_up_arrow (char** list, char* buffer, int* local_history_idx, int* position)
{
    if (*local_history_idx - 1 >= 0)
    {
        erase_buffer(*position);
        strcpy(buffer, list[--(*local_history_idx)]);
        *position = (int) strlen(buffer);
        print_buffer(buffer);
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
        print_buffer(buffer);
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

    pthread_mutex_lock(&ctrl_c_mutex);
    ctrl_c_pressed = 1;    
    pthread_mutex_unlock(&ctrl_c_mutex);

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
    else if (d->trie->matchesCount == 0) 
    {
        return NULL;
    }
    
    printf("\n");
    
    for (int i = 0; i < d->trie->matchesCount; i++)
    {
        printf("%-25s", strrchr(d->trie->matches[i], '/') + 1);
        if ((i + 1) % 3 == 0 && i != d->trie->matchesCount - 1)
        {
            printf("\n");
        }
    }

    return NULL;
}

void 
print_buffer (char* buffer) 
{
    printf("\r> %s", buffer);
    fflush(stdout);
}