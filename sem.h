#ifndef __SEM_H__
#define __SEM_H__
#include <sys/sem.h>
#define SEMKEY 0x7777L
int semid;

int sem_create(key_t key, int initval);
int sem_open(key_t key);
void sem_wait(int id);
void sem_signal(int id);
void sem_op(int id, int amount);
void sem_close(int id);
void sem_rm(int id);

#endif
