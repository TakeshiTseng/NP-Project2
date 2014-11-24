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

    // use share memory to send message!
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

/*
    int msgid = msgget(msg_key, IPC_CREAT | 0666);
    if(msgid == -1) {
        fprintf(stderr, "Can't open queue for client %d\n", to);
        fflush(stderr);
        return -1;
    }

    ras_msg_t data;
    data.type = type;
    data.from = from;
    data.to = to;
    bzero(data.msg, 1024);
    strcpy(data.msg, message);

    mq_msg_t msg;
    msg.type = type;
    msg.msg = data;

    int r = msgsnd(msgid, &msg, sizeof(data), 0);
    if(r < 0) {
        fprintf(stderr, "Can't send msg to client %d\n", to);
        fflush(stderr);
        return -1;
    }
*/
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
    head = (head + 1) % 10;
    set_mbox_info(to, head, tail);
    return data;

/*
    int msgid = msgget(msg_key, IPC_CREAT | 0666);
    if(msgid == -1) {
        fprintf(stderr, "Can't open queue for client %d\n", to);
        fflush(stderr);
        return -1;
    }
    mq_msg_t msg;
    if(type == PIPE_TO_MSG) {
        type = 3000 + from;
    }
    int r = msgrcv(msgid, &msg, sizeof(ras_msg_t), type, IPC_NOWAIT);
    if( r < 0 ) {
        return -1;
    }
    ras_msg_t data = msg.msg;
    strcpy(message, data.msg);
*/
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

/*
    key_t msg_key = 1000 + client_id;
    int msgid = msgget(msg_key, IPC_CREAT | 0666);
    if(msgid == -1) {
        return;
    }
    int r;
    do {
        mq_msg_t tmp;
        r = msgrcv(msgid, &tmp, sizeof(ras_msg_t), -1, IPC_NOWAIT);
    } while(r >= 0);
*/
}
