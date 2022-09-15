#include <stdio.h>
#include <stdlib.h>
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
           printf("\nChild process before kill : PID=%d, GROUP=%d, PPID=%d\n", getpid(), getpgrp(), getppid());
           sleep(SLEEP_TIME);
           printf("\nChild process after kill : PID=%d, GROUP=%d, PPID=%d\n", getpid(), getpgrp(), getppid());
           return EXIT_SUCCESS;
        }
        else
        {
            children_pids[i] = pid;
        }
    }
    printf("Parent process have children with IDs: %d, %d\n", children_pids[0], children_pids[1]);

    return EXIT_SUCCESS;
}
