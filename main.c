#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include "server.h"
#include "main.h"
#include "passivesock.h"
#include "client_node.h"
#include <sys/shm.h>
#include "global_pipe.h"
#include "mq.h"

void handleSIGCHLD() {
    int stat;

    /*Kills all the zombie processes*/
    while(waitpid(-1, &stat, WNOHANG) > 0);
    // while(wait3(&stat, WNOHANG, (struct rusage*)0)>=0);
}

int main(int argc, const char * argv[])
{

    // handling zombie processes
    signal(SIGCHLD, handleSIGCHLD);

    srand(time(0));

    init_client_list();
    init_global_pipe_list();
    init_message_boxes();


    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    int port = 2000 + rand() % 100;
    printf("Port : %d\n", port);
    char port_str[5];
    sprintf(port_str, "%d", port);
    int sc_fd = passivesock(port_str, "tcp", 5);

    printf("accepting.....\n");

    int addrlen = sizeof(client_addr);

    while(1) {
        int new_client_sock = accept(sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
        int pid = fork();

        if(pid == 0) {
            // child
            client_node_t* client = create_client_node(new_client_sock, "CGILAB"/*inet_ntoa(client_addr.sin_addr)*/, 511/*ntohs(client_addr.sin_port)*/);
            close(sc_fd);

            if(insert_to_client_list(client) != -1) {
                serve(client);
                close(new_client_sock);
                break;
            }
        } else {
            close(new_client_sock);
        }
    }
    return 0;
}
void init_client_list() {
    client_id = -1;
    client_semid = -1;
    int shmid = shmget((key_t)CLI_SHM_KEY, sizeof(client_node_t) * 32, IPC_CREAT | 0600);
    if(shmid < 0) {
        perror("init shm for client list error!");
    }

    client_list = (client_node_t*) shmat(shmid, (char*)0, 0);
    int c;
    for(c=0; c<32; c++) {
        client_list[c].id = -1;
    }
}

void init_global_pipe_list() {
    int shmid = shmget((key_t)GPIPE_SHM_KEY, sizeof(pipe_node_t) * MAX_GPIPE, IPC_CREAT | 0600);
    if(shmid < 0) {
        perror("init shm for global node error");
    }
    pipe_node_t* pipe_list = (pipe_node_t*) shmat(shmid, NULL, 0);
    int c;

    for(c=0; c<MAX_GPIPE; c++) {
        pipe_list[c].count = -1;
    }
}

void init_message_boxes() {
    int c;
    for(c=1; c<=30; c++) {
        key_t msg_key = 1000 + c;
        int shmid = shmget(msg_key, sizeof(ras_msg_t) * 10, IPC_CREAT | 0666);
        if(shmid == -1) {
            perror("shmget error");
            return;
        }
        ras_msg_t* message_box = shmat(shmid, NULL, 0);
        if(message_box == NULL) {
            return;
        }
        int cc;
        for(cc=0; cc<10; cc++) {
            message_box[cc].type = -1;
        }
    }

    for(c=1; c<=30; c++) {
        key_t msg_key = 3000 + c;
        int shmid = shmget(msg_key, sizeof(ras_msg_t) * 10, IPC_CREAT | 0666);
        if(shmid == -1) {
            perror("shmget error");
            return;
        }
        ras_msg_t* message_box = shmat(shmid, NULL, 0);
        if(message_box == NULL) {
            return;
        }
        int cc;
        for(cc=0; cc<10; cc++) {
            message_box[cc].type = -1;
        }
    }




}
