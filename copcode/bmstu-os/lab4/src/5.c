#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define SLEEP_TIME 3
#define TRUE 1
#define FALSE 0

int children_pids[2];
const char volatile *messages[] = {"\n1. Первое сообщение\n", "2. SecMes"};
int flag = FALSE;

void catch_signal(int signal)
{
    printf("\nCatched signal: %d\n", signal);
    flag = TRUE;
}

int main(void) 
{
    int fd[2];
    char buf[50] = {0};
    printf("Parent process: PID=%d, GROUP=%d\n", getpid(), getpgrp());

    signal(SIGINT, catch_signal);
    sleep(SLEEP_TIME);
    
    if (pipe(fd) == -1) 
    {
        perror("Can't pipe\n");
        return EXIT_FAILURE;
    }

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
            printf("\nChild process : PID=%d, GROUP=%d, PPID=%d\n", getpid(), getpgrp(), getppid());
            if (flag == TRUE)
            {
                close(fd[0]);
                write(fd[1], messages[i], strlen(messages[i]));
                printf("Message from child has been sent to parent\n");
            }
            exit(EXIT_SUCCESS);
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

    close(fd[1]);
    read(fd[0], buf, sizeof(buf));
    printf("\nReceived messages: %s\n", buf);

    return EXIT_SUCCESS;
}