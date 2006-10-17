#ifndef __pidsync
#define __pidsync

/* for process sync & barrier */

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define LOCAL_SYNCID   0
#define HOST_SYNCID    1
#define ROUTING_SYNCID 2

#define PIDSYNC_KEY_BASE 1300L
#define PIDSYNC_IDMSG    1313L
#define BARRIER_ACKMSG   1314L
#define BARRIER_ZEROMSG  1315L
#define INT_MSG          0L

#ifdef _CRAY
/* There is no usleep on the Cray --> simulate this with select. */
#  include <sys/types.h>
#  include <sys/time.h>
#  define usleep(a)  { struct timeval t;\
                       t.tv_sec=0;\
		       t.tv_usec=(a);\
                       select(0,(fd_set *)0,(fd_set*)0,(fd_set *)0,&t);\
                     }
                     
#endif
#define BACKOFF_TIME_US 10000

#define PIDSYNC_ERROR -1

/* used for synchronising a number N of unknown processes
   via a SYS V message queue */
typedef struct _pidsync_msg {
    long mtype;
    int cnt;
    pid_t pid;
} pidsync_msg;

typedef struct _ack_msg {
    long mtype;
    int id;
} ack_msg;

typedef struct _int_msg {
    long mtype;
    int fromid;        
    int value;
} int_msg;

/* init and close */
int pidopen (int id_offset);
void pidclose (int qid);

/* sync 'numprocs' processes, return my local id. 'id_offset' serves to distinguish
   different synchronization groups */
int pidsync (int numproc, int qid);

/* send and receive integer values */
int pidsnd (int value, int toid, int myid, int qid);
int pidrcv (int fromid, int myid, int qid);

#endif
