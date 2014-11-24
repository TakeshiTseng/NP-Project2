#include "client_node.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "cmd_node.h"
#include <sys/shm.h>
#include "mq.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "ras_sem.h"

client_node_t* get_client_list() {
    if(client_list == NULL) {
        int shmid = shmget((key_t)CLI_SHM_KEY, sizeof(client_node_t) * 32, 0666);
        if(shmid < 0) {
            return NULL;
        }
        client_list = (client_node_t*) shmat(shmid, (char*)0, 0);
    }
    return client_list;
}

client_node_t* create_client_node(int client_sc_fd, char ip[16], int port) {

    client_node_t* new_client_node = malloc(sizeof(client_node_t));

    new_client_node->id = -1;
    new_client_node->client_sc_fd = client_sc_fd;
    bzero(new_client_node->name, 20);
    strncpy(new_client_node->name, "(no name)", 10);
    strncpy(new_client_node->ip, ip, 16);
    new_client_node->port = port;
    new_client_node->cmd_exec_list = NULL;
    new_client_node->pipe_list = NULL;
    bzero(new_client_node->env[0], 1024);
    strcpy(new_client_node->env[0], "PATH");
    bzero(new_client_node->env_val[0], 1024);
    strcpy(new_client_node->env_val[0], "bin:.");
    new_client_node->num_env = 1;
    new_client_node->pid = getpid();
    new_client_node->mbox_head = 0;
    new_client_node->mbox_tail = 0;
    return new_client_node;
}

int insert_to_client_list(client_node_t* new_client_node) {
    client_node_t* client_list = get_client_list();
    int c;
    for(c=1; c<=30; c++) {
        if(client_list[c].id == -1) {
            new_client_node->id = c;
            client_list[c] = *new_client_node;
            return c;
        }
    }
    return -1;
}

void broad_cast(client_node_t* current_client, char* message) {
    client_node_t* client_list = get_client_list();
    int c;
    for(c=1; c<=30; c++) {
        if(client_list[c].id != -1) {
            mq_send_msg(TELL_MSG, current_client->id, c, message);
            kill(client_list[c].pid, SIGUSR1);
            client_wait();
        }
    }
}


void set_env_to_client_node(client_node_t* client, char* name, char* val) {
    int c;
    for(c=0; c<client->num_env; c++) {
        if(strcmp(client->env[c], name) == 0) {
            strcpy(client->env_val[c], val);
            return;
        }
    }
    strcpy(client->env[client->num_env], name);
    strcpy(client->env_val[client->num_env], name);
    client->num_env++;
}

void remove_client_node(client_node_t* client) {
    client_node_t* client_list = get_client_list();
    client_list[client->id].id = -1;
}


int tell(client_node_t* current_client, char* client_id_str, char* message) {
    client_node_t* client_list = get_client_list();
    int client_id = atoi(client_id_str);
    if(client_list[client_id].id == -1 || client_id < 1 || client_id > 30) {
        return -1;
    } else {
        char msg_to_send[2048];
        sprintf(msg_to_send, "*** %s told you ***: %s\n", current_client->name, message);
        mq_send_msg(TELL_MSG, current_client->id, client_id, msg_to_send);
        kill(client_list[client_id].pid, SIGUSR1);
        client_wait();
    }
    return 0;
}

int who(client_node_t* current_client) {
    int c;
    client_node_t* client_list = get_client_list();
    printf("<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
    for(c=1; c<=30; c++) {
        client_node_t t_client = client_list[c];
        if(t_client.id != -1) {
            printf("%d\t%s\t%s/%d", c, t_client.name, t_client.ip, t_client.port);
            if(c == current_client->id) {
                printf("\t<-me");
            }
            printf("\n");
            fflush(stdout);
        }
    }
    return 0;
}

int check_name_exist(char* name) {
    int c;
    client_node_t* client_list = get_client_list();
    for(c=1; c<=30; c++) {
        if(client_list[c].id != -1 && strcmp(client_list[c].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_client_available(int client_id) {
    int res = -1;
    client_node_t* client_list = get_client_list();
    if(client_list[client_id].id != -1) {
        res = 1;
    }
    return res;
}

void change_client_name(int client_id, char* name) {
    client_node_t* client_list = get_client_list();
    strcpy(client_list[client_id].name, name);
}
void get_client_name(int client_id, char* name) {
    client_node_t* client_list = get_client_list();
    strcpy(name, client_list[client_id].name);
}

void get_mbox_info(int client_id, int* head, int* tail) {
    client_node_t* client_list = get_client_list();
    if(client_list[client_id].id == -1) {
        *head = -1;
        *tail = -1;
    } else {
        *head = client_list[client_id].mbox_head;
        *tail = client_list[client_id].mbox_tail;
    }
}

void set_mbox_info(int client_id, int head, int tail) {
    client_node_t* client_list = get_client_list();
    client_list[client_id].mbox_head = head;
    client_list[client_id].mbox_tail = tail;
}

void set_client_name(int client_id, char* name) {
    client_node_t* client_list = get_client_list();
    strcpy(client_list[client_id].name, name);
}

void client_wait() {
    if(client_semid == -1) {
        client_semid = sem_create(CLI_SHM_KEY, 0);
        if(client_semid == -1) {
            perror("create sem error");
            return;
        }
    }
    sem_wait(client_semid);
}

void client_signal() {
    if(client_semid == -1) {
        client_semid = sem_create(CLI_SHM_KEY, 0);
        if(client_semid == -1) {
            perror("create sem error");
            return;
        }
    }
    sem_signal(client_semid);
}
