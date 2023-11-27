#ifndef INPUT_H_SHSH
#define INPUT_H_SHSH

#include "env.h"
#include "history.h"

#include <pthread.h>

#define UP_ARROW 65
#define DOWN_ARROW 66
#define RIGHT_ARROW 67
#define LEFT_ARROW 68

#define BACKSPACE 127

extern volatile int ctrl_c_pressed;

extern pthread_t input_thread;

char *read_line(char *b, int p, ENV *env, History *h);

int get_char(void);

void ctrlC_handler(int signum);

void *input_thread_function(void *arg);

char *tab_completion(char *partial_input, int pos, ENV *env);

#endif