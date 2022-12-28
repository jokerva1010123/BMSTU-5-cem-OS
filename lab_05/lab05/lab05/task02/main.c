#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>

#define WRITERS_NUM 3
#define READERS_NUM 5

#define READER_SLEEP_TIME 2
#define WRITER_SLEEP_TIME 2

#define ACTIVE_WRITERS 0
#define WAITING_WRITERS 1
#define ACTIVE_READERS 2
#define WAITING_READERS 3
#define ACCESS 4

#define ITERS_NUM 7


_Bool flag = true;

void switch_mode(int signal)
{
    flag = false;
}

struct sembuf writer_begin[6] = 
{
    {WAITING_WRITERS, 1, 0},
    {ACTIVE_READERS, 0, 0},
    {ACTIVE_WRITERS, 0, 0},
    {ACTIVE_WRITERS, 1, 0},
    {ACCESS, -1, 0},
    {WAITING_WRITERS, -1, 0}
};

struct sembuf writer_release[2] =
{
    {ACTIVE_WRITERS, -1, 0},
	{ACCESS, 1, 0}
};

struct sembuf reader_begin[5] = 
{
    {WAITING_READERS, 1, 0},
    {ACTIVE_WRITERS, 0, 0},
    {WAITING_WRITERS, 0, 0},
    {ACTIVE_READERS, 1, 0},
    {WAITING_READERS, -1, 0},
};	

struct sembuf reader_release[1] = 
{
    {ACTIVE_READERS, -1, 0}	
};

int start_write(int sem_id) 
{
    return semop(sem_id, writer_begin, 6);
}

int stop_write(int sem_id) 
{
    return semop(sem_id, writer_release, 2);
}

int start_read(int sid) 
{
    return semop(sid, reader_begin, 5);
}

int stop_read(int sid) 
{
    return semop(sid, reader_release, 1);
}

void run_writer(int *const counter, const int sid, const int wid)
{
    srand(time(NULL) + wid);
	int sleep_time = rand() % WRITER_SLEEP_TIME + 1;
	sleep(sleep_time);

	if (start_write(sid) == -1)
	{
		perror("Something went wrong with start_write!");
		exit(-1);
	}

	(*counter)++;
	printf("\e[1;34mWriter #%d write: %2d (sleep: %d)\e[0m\n", 
				wid, *counter, sleep_time);

	if (stop_write(sid) == -1)
	{
		perror("Something went wrong with stop_write!");
		exit(-1);
	}
}

void create_writer(int *const counter, const int sid, const int wid)
{

	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		perror("Can't fork!");
		exit(-1);
	}
	else if (childpid == 0)
	{
        signal(SIGINT, switch_mode);

        while(flag)
			run_writer(counter, sid, wid);

		exit(0);
	}
}

void run_reader(int *const counter, const int sid, const int rid)
{
    srand(time(NULL) + rid);
	int sleep_time = rand() % READER_SLEEP_TIME + 1;
	sleep(sleep_time);

	if (start_read(sid) == -1)
	{
		perror("Something went wrong with start_read!");
		exit(-1);
	}

	printf("\e[1;33mReader #%d  read: %2d (sleep: %d)\e[0m \n", 
				rid, *counter, sleep_time);
	
	if (stop_read(sid) == -1)
	{
		perror("Something went wrong with stop_read!");

        exit(-1);
	}
}

void create_reader(int *const counter, const int sid, const int rid)
{
	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		perror("Can't fork!");
		exit(-1);
	}
	
    if (childpid == 0)
	{
        signal(SIGINT, switch_mode);

        while(flag)
			run_reader(counter, sid, rid);

		exit(0);
	}
}

int main(void)
{
    signal(SIGINT, SIG_IGN);
	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int shmid = shmget(IPC_PRIVATE, sizeof(int), perms);
	if (shmid == -1)
	{
		perror("Failed to create shared memory!");
		return -1;
	}

    int *counter = (int*)shmat(shmid, NULL, 0);
	if (counter == (void*)-1)
	{
		perror("Shmat failed!");
		return -1;
	}
    *counter = 0;

    int sem_descr = semget(IPC_PRIVATE, 5, IPC_CREAT | perms);
	if (sem_descr == -1)
	{
		perror("Failed to create semaphores!");
		return -1;
	}

	if (semctl(sem_descr, ACTIVE_READERS, SETVAL, 0) == -1)
	{
		perror("Can't set control semaphors!");
		return -1;
	}

	if (semctl(sem_descr, ACTIVE_WRITERS, SETVAL, 0) == -1)
	{
		perror("Can't set control semaphors!");
		return -1;
	}

	if (semctl(sem_descr, WAITING_WRITERS, SETVAL, 0) == -1)
	{
		perror("Can't set control semaphors!");
		return -1;
	}

    if (semctl(sem_descr, WAITING_READERS, SETVAL, 0) == -1)
	{
		perror("Can't set control semaphors!");
		return -1;
	}

	if (semctl(sem_descr, ACCESS, SETVAL, 1) == -1)
	{
		perror("Can't set control semaphors!");
		return -1;
	}

	for (int i = 0; i < WRITERS_NUM; i++)
		create_writer(counter, sem_descr, i + 1);

    for (int i = 0; i < READERS_NUM; i++)
		create_reader(counter, sem_descr, i + 1);

	for (int i = 0; i < READERS_NUM + WRITERS_NUM; i++)
    {
        int status;
        if (wait(&status) == -1) 
		{
            perror("Something wrong with children waiting!");
            return -1;
        }

        if (!WIFEXITED(status))
            printf("One of children terminated abnormally!");
    }

    printf("\e[1;32mOK\e[0m\n");

	if (shmctl(shmid, IPC_RMID, NULL) == -1)
	{
		perror("Failed to mark the segment to be destroyed!");
		return -1;
	}

	if (shmdt((void*)counter) == -1)
	{
		perror("Failed to detach the segment!");
		return -1;
	}

	if (semctl(sem_descr, 0, IPC_RMID, 0) == -1)
	{
		perror("Failed to delete semaphores!");
		return -1;
	}

	return 0;
}

