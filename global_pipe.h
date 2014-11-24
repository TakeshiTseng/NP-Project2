#ifndef __GLOBAL_PIPE_H__
#define __GLOBAL_PIPE_H__

#define GPIPE_SHM_KEY (key_t)0x5660L
#define PIPE_SEM_KEY (key_t)0x5661L
#define MAX_GPIPE 10000


int pipe_semid;

int is_global_pipe_exist(int from , int to);
void add_global_pipe(int from, int to);
void remove_global_pipe(int from, int to, int both_match);

#endif
