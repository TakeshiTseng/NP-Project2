#include "ras_sem.h"
#include <sys/errno.h>
#include <sys/sem.h>
#include <stdio.h>

#define BIGCOUNT 100

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

struct sembuf op_endcreate[2] = {
    1, -1, SEM_UNDO,
    2, -1, SEM_UNDO
};

static struct sembuf op_lock[2] = {
    2, 0, 0,
    2, 1, SEM_UNDO
};

static struct sembuf op_open[1] = {
    1, -1, SEM_UNDO /* decrement [1] (proc counter) with undo on exit */
};

static struct sembuf op_close[3] = {
    2, 0, 0, /* wait for [2] (lock) to equal 0 */
    2, 1, SEM_UNDO,/* then increment [2] to 1 - this locks it */
    1, 1, SEM_UNDO /* then increment [1] (proc counter) */
};

static struct sembuf op_unlock[1] = {
    2, -1, SEM_UNDO /* decrement [2] (lock) back to 0 */
};

static struct sembuf op_op[1] = {
    0, 99, SEM_UNDO
};

int sem_create(key_t key, int initval)
{
    register int id, semval;
    union semun semctl_arg;
    if (key == IPC_PRIVATE) return(-1); /* not intended for private semaphores */
    else if (key == (key_t) -1) return(-1); /* probably an ftok() error by caller */
again:
    if ( (id = semget(key, 3, 0666 | IPC_CREAT)) < 0) return(-1);
    if (semop(id, &op_lock[0], 2) < 0) {
        if (errno == EINVAL) goto again;
        perror("can't lock");
    }
    if ( (semval = semctl(id, 1, GETVAL, 0)) < 0) perror("can't GETVAL");
    if (semval == 0) {
        semctl_arg.val = initval;
        if (semctl(id, 0, SETVAL, semctl_arg) < 0)
            perror("can’t SETVAL[0]");
        semctl_arg.val = BIGCOUNT;
        if (semctl(id, 1, SETVAL, semctl_arg) < 0)
            perror("can’t SETVAL[1]");
    }
    if (semop(id, &op_endcreate[0], 2) < 0)
        perror("can't end create");
    return(id);
}


void sem_rm(int id)
{
    if (semctl(id, 0, IPC_RMID, 0) < 0)
        perror("can't IPC_RMID");
}

int sem_open(key_t key)
{
    register int id;
    if (key == IPC_PRIVATE) return(-1);
    else if (key == (key_t) -1) return(-1);
    if ( (id = semget(key, 3, 0)) < 0) return(-1); /* doesn't exist, or tables full */

    if (semop(id, &op_open[0], 1) < 0) perror("can't open");
    return(id);
}


void sem_close(int id)
{
    register int semval;

    if (semop(id, &op_close[0], 3) < 0) perror("can't semop");

    if ( (semval = semctl(id, 1, GETVAL, 0)) < 0) perror("can't GETVAL");
    if (semval > BIGCOUNT) perror("sem[1] > BIGCOUNT");
    else if (semval == BIGCOUNT) sem_rm(id);
    else
        if (semop(id, &op_unlock[0], 1) < 0) perror("can't unlock"); /* unlock */
}

void sem_op(int id, int value)
{
    if ( (op_op[0].sem_op = value) == 0) perror("can't have value == 0");
    if (semop(id, &op_op[0], 1) < 0) perror("sem_op error");
}

void sem_wait(int id)
{
    sem_op(id, -1);
}


void sem_signal(int id)
{
    sem_op(id, 1);
}
