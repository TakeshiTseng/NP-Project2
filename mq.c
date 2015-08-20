#include "mq.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "client_node.h"
#include <unistd.h>


void _dbg_print_all(int client_id) {
    key_t msg_key = 1000 + client_id;
    int shmid = shmget(msg_key, sizeof(ras_msg_t) * 10, IPC_CREAT | 0666);
    if(shmid == -1) {
        perror("shmget error");
        return;
    }
    ras_msg_t* message_box = shmat(shmid, NULL, 0);
    if(message_box == NULL) {
        return;
    }

    int c;
    // check if mail box is full
    for(c=0; c<10; c++) {
        printf("[DBG] mbox: type: %d, from: %d, to: %d, msg: %s\n",
                message_box[c].type, message_box[c].from, message_box[c].to, message_box[c].msg);
        fflush(stdout);
    }

}

int mq_send_msg(int type, int from, int to, char* message) {
    key_t msg_key = 1000 + to;
    int shmid = shmget(msg_key, sizeof(ras_msg_t) * 10, IPC_CREAT | 0666);
    if(shmid == -1) {
        perror("shmget error");
        return -1;
    }
    ras_msg_t* message_box = shmat(shmid, NULL, 0);
    if(message_box == NULL) {
        return -1;
    }


    int head, tail;
    get_mbox_info(to, &head, &tail);

    if(tail == head && message_box[tail].type != -1) {
        // full
        return -1;
    }

    message_box[tail].type = type;
    message_box[tail].from = from;
    message_box[tail].to = to;
    strcpy(message_box[tail].msg, message);
    tail = (tail + 1) % 10;
    set_mbox_info(to, head, tail);


    return 0;
}

ras_msg_t mq_recv_msg(int type, int from, int to) {
    key_t msg_key = 1000 + to;
    ras_msg_t data;
    data.type = -1;
    int shmid = shmget(msg_key, sizeof(ras_msg_t) * 10, IPC_CREAT | 0666);
    if(shmid == -1) {
        perror("shmget error");
        return data;
    }
    ras_msg_t* message_box = shmat(shmid, NULL, 0);
    if(message_box == NULL) {
        return data;
    }

    int head, tail;
    get_mbox_info(to, &head, &tail);
    if(head == tail) {
        // empty
        return data;
    }
    data = message_box[head];
    message_box[head].type = -1;
    head = (head + 1) % 10;
    set_mbox_info(to, head, tail);
    return data;

}

void mq_clean(int client_id) {
    key_t msg_key = 1000 + client_id;
    int shmid = shmget(msg_key, sizeof(ras_msg_t) * 10, IPC_CREAT | 0666);
    if(shmid == -1) {
        perror("shmget error");
        return;
    }
    ras_msg_t* message_box = shmat(shmid, NULL, 0);
    if(message_box == NULL) {
        return;
    }
    int c;
    for(c=0; c<10; c++) {
        message_box[c].type = -1;
    }
    set_mbox_info(client_id, 0, 0);

}
