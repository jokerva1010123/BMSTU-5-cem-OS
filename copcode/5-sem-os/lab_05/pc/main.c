#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define err_semgrt_msg "ERROR while creating semaphores\n"
#define err_semctl_msg "ERROR while taking under control semaphores\n"
#define err_semop_msg "ERROR while making operation on semaphores\n"
#define err_shmget_msg "ERROR while creating shared memory\n"
#define err_shmat_msg "ERROR while attaching shared memory\n"
#define err_shmdt_msg "ERROR while detaching shared memory\n"
#define err_fork_msg "ERROR fork\n"

const int N = 10;
const int PERM = S_IRWXU | S_IRWXG | S_IRWXO;

int* shared_buffer;
int* sh_pos_cons;
int* sh_pos_prod;
int* letter;

#define SE 0
#define SF 1
#define SB 2

#define P -1
#define V 1

struct sembuf prod_beg_operations[2] = { {SE, P, 0}, {SB, P, 0} };
struct sembuf prod_end_operations[2] =  { {SB, V, 0}, {SF, V, 0} };
struct sembuf cons_beg_operations[2] = { {SF, P, 0}, {SB, P, 0} };
struct sembuf cons_end_operations[2] =  { {SB, V, 0}, {SE, V, 0} };


void producer(const int semid, const int value)
{
    sleep(rand() % 5);
    int sem_op_p = semop(semid, prod_beg_operations, 2);
    if ( sem_op_p == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }


    shared_buffer[*sh_pos_prod] = *letter;
    printf("Producer #%d: value %c was moved into buffer at %d\n", value, shared_buffer[*sh_pos_prod], *sh_pos_prod);
    (*sh_pos_prod)++;
    (*letter)++;

    int sem_op_v = semop(semid, prod_end_operations, 2);
    if ( sem_op_v == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }
}

void consumer(const int semid, const int value)
{
    sleep(rand() % 2);
    int sem_op_p = semop(semid, cons_beg_operations, 2);
    if ( sem_op_p == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }

    printf("Consumer #%d: value %c was read from buffer at %d\n", value, shared_buffer[*sh_pos_cons], *sh_pos_cons);

    (*sh_pos_cons)++;

    int sem_op_v = semop(semid, cons_end_operations, 2);
    if ( sem_op_v == -1 )
    {
        perror(err_semop_msg);
        exit( 1 );
    }

}

int main()
{
    int shmid, semid;


    if ((shmid = shmget(IPC_PRIVATE, (N + 3) * sizeof(int), IPC_CREAT | PERM)) == -1)
    {
        perror(err_shmget_msg);
        exit( 1 );
    }

    // При исполнении fork() дочернии процесс наследует пристыкованные
    // сегменты разделяемой памяти.
    shared_buffer = shmat(shmid, 0, 0);
    if (shared_buffer == -1)
    {
        perror(err_shmat_msg);
        exit( 1 );
    }

    sh_pos_prod = shared_buffer + N * sizeof(int);
    sh_pos_cons = shared_buffer + (N + 1) * sizeof(int);
    letter = shared_buffer + (N + 2) * sizeof(int);
    *letter = 65;

    // IPC_PRIVATE- не поле флага, а тип key_t.
    // Если key равно этому значению, то системный вызов игнорирует все,
    // кроме 9-ти младших битов semflg и создает новый набор семафоров.
    if ((semid = semget(IPC_PRIVATE, 3, IPC_CREAT | PERM)) == -1)
    {
        perror(err_semgrt_msg);
        exit( 1 );
    }

    int ctrl_sb = semctl(semid, SB, SETVAL, 1);
    int ctrl_se = semctl(semid, SE, SETVAL, N);
    int ctrl_sf = semctl(semid, SF, SETVAL, 0);

    if ( ctrl_se == -1 || ctrl_sf == -1 || ctrl_sb == -1)
    {
        perror(err_semctl_msg);
        exit( 1 );
    }


    pid_t pid = -1;
    for (int i = 0; i < N && pid != 0; i++) {
        pid = fork();
        if (pid == -1) {
            perror(err_fork_msg);
            exit( 1 );
        }
        if (pid == 0) {
            producer(semid, i);
        }
    }

    for (int i = 0; i < N && pid != 0; i++) {
        pid = fork();
        if (pid == -1) {
            perror(err_fork_msg);
            exit( 1 );
        }
        if (pid == 0) {
            consumer(semid, i);
        }
    }

    if (pid != 0) {
        int *status;
        for (int i = 0; i < N + N; ++i) {
            wait(status);
        }
        if (shmdt(shared_buffer) == -1)
        {
            perror(err_shmdt_msg);
            exit( 1 );
        }
    }
}
