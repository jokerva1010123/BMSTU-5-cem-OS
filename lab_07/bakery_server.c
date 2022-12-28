/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "bakery.h"

typedef struct target
{
	int num; // очередь
	int res;
	int id;  
} target_t;

int choosing[MAX_CLIENT] = { 0 };
int number[MAX_CLIENT] = { 0 };
int symbol = 1;
int num = 0;

void *get_number(void *arg)
{
	target_t *targ = arg;
	int i = num;
	num++;
	targ->id = i;
	choosing[i] = 1;
	int max = 0;

	for (int j = 0; j < MAX_CLIENT; j++)
		if (number[j] > max)
			max = number[j];

	number[i] = max + 1;
	targ->num = number[i];
	choosing[i] = 0;
}

void *bakery(void *arg)
{
	target_t *targ = arg;
	int i = targ->id;

	for (int j = 0; j < MAX_CLIENT; j++) 
	{
		while (choosing[j]);
		while ((number[j] > 0) && (number[j] < number[i] || 
		(number[j] == number[i] && j < i)));
	}
	
	targ->res = symbol;
	symbol++;
	number[i] = 0;

	return 0;
}

struct BAKERY *
bakery_proc_1_svc(struct BAKERY *argp, struct svc_req *rqstp)
{
	static struct BAKERY  result;
	switch (argp->op)
	{
		case GET_NUMBER:
		{
			pthread_t thread;
			target_t tres;
			pthread_create(&thread, NULL, get_number, &tres);
			pthread_join(thread, NULL);
			result.pid = argp->pid;
			result.num = tres.num;
			result.op = tres.id;
			break;
		}	
		case ENTER_CRIT_SECTION:	
		{
			pthread_t thread;
			target_t tres;
			tres.id = argp->num;
			pthread_create(&thread, NULL, bakery, &tres);
			pthread_join(thread, NULL);
			result.pid = argp->pid;
			result.result = tres.res;
			result.op = tres.id;
			break;
		}
		default:
			break;

	}

	return &result;
}