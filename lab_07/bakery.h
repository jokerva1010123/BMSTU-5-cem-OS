#ifndef _BAKERY_H_RPCGEN
#define _BAKERY_H_RPCGEN

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#define GET_NUMBER 0
#define ENTER_CRIT_SECTION 1
#define MAX_CLIENT 5

struct BAKERY {
	int op;
	int num;
	int pid;
	int result;
};
typedef struct BAKERY BAKERY;


struct to_fork
{
	CLIENT *client;
	int i;
    pid_t *children;
};


#define BAKERY_PROG 0x20000001
#define BAKERY_VER 1

#if defined(__STDC__) || defined(__cplusplus)
#define BAKERY_PROC 1
extern  struct BAKERY * bakery_proc_1(struct BAKERY *, CLIENT *);
extern  struct BAKERY * bakery_proc_1_svc(struct BAKERY *, struct svc_req *);
extern int bakery_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define BAKERY_PROC 1
extern  struct BAKERY * bakery_proc_1();
extern  struct BAKERY * bakery_proc_1_svc();
extern int bakery_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_BAKERY (XDR *, BAKERY*);

#else /* K&R C */
extern bool_t xdr_BAKERY ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_BAKERY_H_RPCGEN */