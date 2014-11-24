#include "mq.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

int mq_send_msg(int type, int from, int to, char* message) {
    key_t msg_key = 1000 + to;
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
    return 0;
}

int mq_recv_msg(int type, int from, int to, char* message) {
    key_t msg_key = 1000 + to;
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
    return 0;
}

void mq_clean(int client_id) {
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

}
