#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SLEEP_TIME 3

int children_pids[2];

int main(void) 
{
    printf("Parent process: PID=%d, GROUP=%d\n", getpid(), getpgrp());

    for (size_t i = 0; i < 2; i++) 
    {
    	int pid; 
    	if ((pid  = fork()) == -1)
        {
           perror("Can't fork\n");
           return EXIT_FAILURE;
        }
        else if (pid == 0)
        {
           sleep(SLEEP_TIME);
           printf("\nChild process : PID=%d, GROUP=%d, PPID=%d\n", getpid(), getpgrp(), getppid());
           return EXIT_SUCCESS;
        }
        else
        {
            children_pids[i] = pid;
        }
    }

    for (size_t i = 0; i < 2; i++) 
    {
        int status;
        pid_t childpid = wait(&status);
        printf("\n\nChild process finished: PID = %d, status = %d\n", childpid, status);

        if (WIFEXITED(status)) 
        {
            printf("Дочерний процесс завершён корректно.\n");
            printf("Child process exited with code %d\n", WEXITSTATUS(status));
        } 
        else if (WIFSIGNALED(status))
        {
            printf("Дочерний процесс завершен неперехватываемым сигналом\n");
            printf("Номер сигнала: \t%d\n\n", WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf ("Дочерний процесс остановлен\n");
            printf ("Номер сигнала: \t%d\n\n", WSTOPSIG (status));
        }
    }

    return EXIT_SUCCESS;
}
