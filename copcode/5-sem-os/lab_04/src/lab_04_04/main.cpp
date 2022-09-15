#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "string"
#define ERROR_FORK 1
#define ERROR_EXEC 2
#define ERROR_PIPE 3
#define OK 0
int main()
{
    int fd[2];
    const char *messages[2] = { "msg\n", "msg msg\n"};
    int len[2] = {strlen(messages[0]), strlen(messages[1])};
    if (pipe(fd) == -1) {
        return ERROR_PIPE;
    }
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
            close(fd[0]);
            write(fd[1], messages[i], len[i]);
            printf("CHILD №%d (pid: %d, ppid: %d, grp: %d) sent message to parent\n", i + 1, getpid(), getppid(), getpgrp());
            return OK;
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
            printf("PARENT:child №%d (PID = %d) has finished because of signal: %d\n", i + 1, childpid, WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf("PARENT: child №%d (PID = %d) has been stopped because of signal: %d\n", i + 1, childpid, WSTOPSIG(status));
        }
    }
    char buf[len[0] + len[1]];
    close(fd[1]);
    read(fd[0], buf, len[0] + len[1]);
    buf[len[0] + len[1]] = '\0';
    printf("PARENT: received messages:\n%s", buf);
    return OK;
}