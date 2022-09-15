#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define err_semgrt_msg "ERROR while creating semaphores\n"
#define err_semctl_msg "ERROR while taking under control semaphores\n"
#define err_semop_msg "ERROR while making operation on semaphores\n"
#define err_shmget_msg "ERROR while creating shared memory\n"
#define err_shmat_msg "ERROR while attaching shared memory\n"
#define err_shmdt_msg "ERROR while detaching shared memory\n"
#define err_fork_msg "ERROR fork\n"

#define WRITERS 10
#define READERS 20

#define READERS_Q 0
#define WRITERS_Q 1
#define W_ACCESS 2

const int PERM =  S_IRWXU | S_IRWXG | S_IRWXO;

struct sembuf start_read[] = {  { WRITERS_Q,  0, 0 },
                                { READERS_Q,  1, 0 } };
struct sembuf  stop_read[] = { {READERS_Q, -1, 0} };
struct sembuf  start_write[] = { { READERS_Q,      0, 0 },
                                 { W_ACCESS,      -1, 0 },
                                 { WRITERS_Q,      1, 0 } };
struct sembuf  stop_write[] = { { WRITERS_Q,    -1, 0 },
                                { W_ACCESS,      1, 0 }};

void writer(int semid, int* shm, int num)
{
    sleep(rand() % 3);
    int sem_op_p = semop(semid, start_write, 3);
    if ( sem_op_p == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }

    (*shm)++;
    printf("Writer #%d: wrote into buffer value %d\n", num, *shm);

    sem_op_p = semop(semid, stop_write, 2);
    if ( sem_op_p == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }
}

void reader(int semid, int* shm, int num)
{
    sleep(rand() % 3);
    int sem_op_p = semop(semid, start_read, 2);
    if ( sem_op_p == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }

    printf("Reader #%d: read from buffer value %d\n", num, *shm);

    sem_op_p = semop(semid, stop_read, 1);
    if ( sem_op_p == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }
}

int main() {

    srand( time( NULL ) );

    int shm_id;
    if ((shm_id = shmget(IPC_PRIVATE, 1 * sizeof(int), IPC_CREAT | PERM)) == -1)
    {
        perror(err_shmget_msg);
        exit( 1 );
    }

    int *shm_buf = shmat(shm_id, 0, 0);
    if (shm_buf == (void*) -1)
    {
        perror(err_shmat_msg);
        exit( 1 );
    }

    (*shm_buf) = 0;

    int sem_id;
    if ((sem_id = semget(IPC_PRIVATE, 3, IPC_CREAT | PERM)) == -1)
    {
        perror(err_semgrt_msg);
        exit( 1 );
    }

    int ctrl_ar = semctl(sem_id, READERS_Q, SETVAL, 0);
    int ctrl_aw = semctl(sem_id, WRITERS_Q, SETVAL, 0);
    int ctrl_acc = semctl(sem_id, W_ACCESS, SETVAL, 1);

    if ( ctrl_ar == -1 || ctrl_aw == -1 || ctrl_acc)
    {
        perror(err_semctl_msg);
        exit( 1 );
    }

    pid_t pid = -1;

    for (int i = 0; i < WRITERS && pid != 0; i++) {
        pid = fork();
        if (pid == -1) {
            perror(err_fork_msg);
            exit( 1 );
        }
        if (pid == 0) {
            writer(sem_id, shm_buf, i);
        }
    }

    for (int i = 0; i < READERS && pid != 0; i++) {
        pid = fork();
        if (pid == -1) {
            perror(err_fork_msg);
            exit( 1 );
        }
        if (pid == 0) {
            reader(sem_id, shm_buf, i);
        }
    }


    if (pid != 0) {
        int *status;
        for (int i = 0; i < WRITERS + READERS; ++i) {
            wait(status);
        }
        if (shmdt(shm_buf) == -1) {
            perror(err_shmdt_msg);
            exit( 1 );
        }
    }

    return 0;
}
