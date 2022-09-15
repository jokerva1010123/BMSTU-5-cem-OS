#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define ERROR_FORK 1
#define ERROR_EXEC 2
#define OK 0
int main()
{
    int childpids[2];
    for (int i = 0; i < 2; i++)
    {
        int pid = fork();

        if (pid == -1)
        {
            return ERROR_FORK;
        }

        if (pid == 0)
        {
            printf("CHILD №%d: pid: %d, ppid: %d, grp: %d\n", i + 1, getpid(), getppid(), getpgrp());
            if (i == 0) execl("sort.app", "sort", "1", "3", "2", "0", "4", "5", NULL);
            else execl("max.app", "max", "1", "3", "2", "0", "4", "5", NULL);
            return ERROR_EXEC;
        }
        childpids[i] = pid;
    }
    printf("PARENT: pid: %d grp: %d, child's pids: %d, %d\n", getpid(), getpgrp(), childpids[0], childpids[1]);
    for (int i = 0; i < 2; i++)
    {
        int status;
        pid_t childpid = wait(&status);
        if (WIFEXITED(status))
        {
            printf("PARENT: child №%d (PID = %d) has finished with code: %d\n", i + 1, childpid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("PARENT: child №%d (PID = %d) has finished because of signal: %d\n", i + 1, childpid, WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf("PARENT: child №%d (PID = %d) has been stopped because of signal: %d\n", i + 1, childpid, WSTOPSIG(status));
        }
    }
    return OK;
}