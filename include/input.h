#ifndef SHSH_INPUT_H
#define SHSH_INPUT_H

#include "env.h"
#include "history.h"

#include <pthread.h>

#define UP_ARROW 65
#define DOWN_ARROW 66
#define RIGHT_ARROW 67
#define LEFT_ARROW 68

#define ESCAPE 27
#define BACKSPACE 127

#define OPEN_BRACKET 91

#define CTRL_L 0x0C

#define SKIP_CWD 0xF

extern volatile int ctrl_c_pressed;

extern pthread_t input_thread;

char *read_line(char *b, int p, ENV *env, History *h, int flag);

void erase_buffer(int count);

int get_char(void);

void handle_arrow(int arrow_key, History *history, char *buffer, int *local_history_idx, int *position);

void handle_up_arrow(char **list, char *buffer, int *local_history_idx, int *position);

void handle_down_arrow(char **list, char *buffer, int *local_history_idx, int *position, int limit);

void handle_right_arrow(int *position, int buffer_length);

void handle_left_arrow(int *position);

void ctrlC_handler(int signum);

void ctrlL_handler(int signum);

void *input_thread_function(void *arg);

char *tab_completion(char *partial_input, int pos, ENV *env);

void print_buffer(char *buffer);

#endif
