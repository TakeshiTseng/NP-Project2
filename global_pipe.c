#include "global_pipe.h"
#include "pipe_node.h"
#include "sem.h"
#include <sys/shm.h>
#include <stdio.h>


pipe_node_t* get_global_pipe_list() {
    int shmid = shmget((key_t)GPIPE_SHM_KEY, sizeof(pipe_node_t) * MAX_GPIPE, 0666);
    if(shmid < 0) {
        return NULL;
    }
    pipe_node_t* pipe_node_list = (pipe_node_t*) shmat(shmid, NULL, 0);
    return pipe_node_list;
}

int is_global_pipe_exist(int from , int to) {
    int res = 0;
    pipe_node_t* pipe_list = get_global_pipe_list();
    int c;
    for(c=0; c<MAX_GPIPE; c++) {
        if(pipe_list[c].count != -1 && pipe_list[c].from_user_id == from && pipe_list[c].to_user_id == to) {
            res = 1;
            break;
        }
    }
    shmdt(pipe_list);
    return res;
}

void add_global_pipe(int from, int to) {
    pipe_node_t* pipe_list = get_global_pipe_list();
    int c;
    for(c=0; c<MAX_GPIPE; c++) {
        if(pipe_list[c].count == -1) {
            pipe_list[c].count = 1;
            pipe_list[c].from_user_id = from;
            pipe_list[c].to_user_id = to;
            break;
        }
    }
    shmdt(pipe_list);

}

void remove_global_pipe(int from, int to, int both_match) {
    pipe_node_t* pipe_list = get_global_pipe_list();

    if(both_match == 1) {
        int c;
        for(c=0; c<MAX_GPIPE; c++) {
            if(pipe_list[c].count != -1 && pipe_list[c].from_user_id == from && pipe_list[c].to_user_id == to) {
                pipe_list[c].count = -1;
                break;
            }
        }
    } else {
        int c;
        for(c=0; c<MAX_GPIPE; c++) {
            if(pipe_list[c].count != -1 && (pipe_list[c].from_user_id == from || pipe_list[c].to_user_id == to)) {
                pipe_list[c].count = -1;
            }
        }
    }
    shmdt(pipe_list);

}

