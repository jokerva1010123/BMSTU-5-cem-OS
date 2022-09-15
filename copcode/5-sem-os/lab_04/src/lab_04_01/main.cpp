#include <stdio.h>
#include <unistd.h>

#define ERROR_FORK 1
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
            printf("\nCHILD №%d BEFORE BLOCK: pid: %d, ppid: %d, grp: %d\n", i + 1, getpid(), getppid(), getpgrp());
            sleep(2);
            printf("\nCHILD №%d AFTER BLOCK: pid: %d, ppid: %d, grp: %d\n", i + 1, getpid(), getppid(), getpgrp());
            return OK;
        }

        childpids[i] = pid;
    }
    printf("PARENT: pid: %d grp: %d, child's pids: %d, %d\n", getpid(), getpgrp(), childpids[0], childpids[1]);

    sleep(1);

    return OK;
}