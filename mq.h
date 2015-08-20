#ifndef __MQ_H__
#define __MQ_H__


// message type
#define TELL_MSG 1
#define PIPE_TO_MSG 2


struct ras_msg {
    int type;
    int from;
    int to;
    char msg[2048];
};

typedef struct ras_msg ras_msg_t;

struct mq_msg {
    long type;
    ras_msg_t msg;
};

typedef struct mq_msg mq_msg_t;

int mq_send_msg(int type, int from, int to, char* message);
ras_msg_t mq_recv_msg(int type, int from, int to);
void mq_clean(int client_id);


#endif

