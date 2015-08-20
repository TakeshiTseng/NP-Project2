#ifndef __GLOBAL_PIPE_H__
#define __GLOBAL_PIPE_H__
#include "pipe_node.h"

#define GPIPE_SHM_KEY (key_t)0x56600L
#define PIPE_SEM_KEY (key_t)0x56610L
#define MAX_GPIPE 10000


int pipe_semid;

int is_global_pipe_exist(int from , int to);
void add_global_pipe(int from, int to, char* message);
void remove_global_pipe(int from, int to, int both_match);
int pull_global_pipe_data(int from, int to, char* data);

#endif
