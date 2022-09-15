#include <stdio.h>
#include <unistd.h>

int main()
{
    int childpid;

    if ((childpid = fork())== -1)
    {
        perror("Can’t fork.");
        return 1;
    }
    else if (childpid == 0)
    {
        while (1)
        {
            printf("%d - i am child", getpid());
        }
    }
    else
    {
        while(1)
        {
            printf("%d - i am parent", getpid());
        }
    }

    return 0;
}