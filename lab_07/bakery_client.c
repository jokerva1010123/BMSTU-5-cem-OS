#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "bakery.h"

void *fork_client(void *temp_struct)
{
	struct BAKERY *response;
	struct BAKERY bakery_proc_1_arg;

	struct to_fork *my_struct = (struct to_fork*) temp_struct;

	CLIENT *clnt = my_struct->client; 
	int *children = my_struct->children;
	int i = my_struct->i;

	if ((my_struct->children[i] = fork()) == -1)
	{
		perror("Can't fork\n");

		exit(1);
	}
	else if (children[i] == 0)
	{
		srand(time(NULL));
		int interval = rand() % 3 + 1;
	
		bakery_proc_1_arg.op = GET_NUMBER;
		response = bakery_proc_1(&bakery_proc_1_arg, clnt);

		printf("\nClient pid = %d queue %d \n", getpid(), response->num);
		
		if (response == (struct BAKERY *) NULL) 
		{
			clnt_perror(clnt, "bakery_proc_1 failed");
		}

		 sleep(interval);
		
		bakery_proc_1_arg.op = ENTER_CRIT_SECTION; 
		bakery_proc_1_arg.num = response->op;
		bakery_proc_1_arg.pid = getpid();
		response = bakery_proc_1(&bakery_proc_1_arg, clnt);

		if (response == (struct BAKERY *) NULL) 
		{
			clnt_perror(clnt, "bakery_proc_1 failed");
		}

		printf("%d get %d (sleep = %d)\n", response->pid, response->result, interval);
		clnt_destroy(clnt);
	}
	else
	{
		int status;
		waitpid(children[i], &status, 0);
	}	
}


void bakery_prog_1(char *host)
{
	CLIENT *clnt;
	struct BAKERY *response;
	struct BAKERY bakery_proc_1_arg;
	
	clnt = clnt_create(host, BAKERY_PROG, BAKERY_VER, "udp");

	if (clnt == NULL) 
	{
		clnt_pcreateerror(host);
		exit(1);
	}

	srand(time(NULL));
	int interval = rand() % 3 + 1;
	sleep(interval);

	bakery_proc_1_arg.op = GET_NUMBER;
	response = bakery_proc_1(&bakery_proc_1_arg, clnt);

	printf("\nClient pid = %d queue %d \n", getpid(), response->num);
	
	if (response == (struct BAKERY *) NULL) 
	{
		clnt_perror(clnt, "bakery_proc_1 failed");
	}

	sleep(interval);
	
	bakery_proc_1_arg.op = ENTER_CRIT_SECTION; 
	bakery_proc_1_arg.num = response->op;
	bakery_proc_1_arg.pid = getpid();
	response = bakery_proc_1(&bakery_proc_1_arg, clnt);

	if (response == (struct BAKERY *) NULL) 
	{
		clnt_perror(clnt, "bakery_proc_1 failed");
	}

	printf("%d get %d (sleep = %d)\n", response->pid, response->result, interval);
	clnt_destroy(clnt);
}

int main (int argc, char *argv[])
{
	setbuf(stdin, NULL);
	char *host;

	if (argc < 2) 
	{
		printf("usage: %s server_host\n", argv[0]);
		exit(1);
	}

	host = argv[1];
	printf("\n");
	bakery_prog_1(host);
	exit(0);
}