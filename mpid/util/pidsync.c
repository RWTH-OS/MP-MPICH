#include <stdlib.h>
#include "pidsync.h"
#ifdef _CRAY
/* for access to MPIR_meta_cfg, which is needed for the cray implementation */
#include "metampi.h"
#endif

#ifdef linux

#define MSG_R   0400    /* read permission */
#define MSG_W   0200    /* write permission */
#endif


/*
 * minimal msg-passing interface based on IPC-Msg-API 
 */

/* pidopen()  */
int pidopen (int id_offset)
{
#ifdef _CRAY
     return id_offset;
#else
    int queue_key = PIDSYNC_KEY_BASE + id_offset;
    int queue_id;

    /* create msgqueue */
    if ((queue_id = msgget (queue_key, IPC_CREAT | 
			    (MSG_R >> 6) | (MSG_R >> 3) | (MSG_R) | 
			    (MSG_W >> 6) | (MSG_W >> 3) | (MSG_W)))  < 0) {
	printf ("pidsync: Couldn't install msgqueue \n");
	return PIDSYNC_ERROR;
    }

    return (queue_id);
#endif
}

/* pidclose()  */
void pidclose (int qid)
{
#ifndef _CRAY
    msgctl (qid, IPC_RMID, 0); 
#endif
}

/* pidsync()  */
int pidsync (int numprocs, int qid)
{
#ifdef _CRAY
    shmem_barrier_all();
    if (qid==LOCAL_SYNCID  ) return _my_pe();
    if (qid==HOST_SYNCID   ) return _my_pe();
    if (qid==ROUTING_SYNCID) 
      return _my_pe() -  MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank];
#else
    pidsync_msg pidmsg;
    ack_msg ackmsg;    
    struct msqid_ds qinfo;

    int i;
    int *pids, npids, mypid, myid, minpid;

    pids = (int *)malloc(numprocs*sizeof(int));
    mypid = getpid(); 
    pids[0] = mypid;
    npids = 1;
    for (i = 1; i < numprocs; i++)
	pids[i] = 0;
    
    /* put own pid into the queue */
    pidmsg.mtype = PIDSYNC_IDMSG;
    pidmsg.cnt   = 1;
    pidmsg.pid   = pids[0];
    if (msgsnd (qid, (void *)&pidmsg, sizeof(pidsync_msg) - sizeof(long), 0) < 0) {
	printf ("pidsync:  msgsnd() [1] failed with errno = %d\n", errno);
	return PIDSYNC_ERROR;
    }

    /* retrieve other pids from queue */
    while (npids != numprocs) {
	if ((i = msgrcv (qid, (void *)&pidmsg, sizeof(pidsync_msg) - sizeof(long), PIDSYNC_IDMSG, 0)) < 0) {
	    printf ("pidsync: msgrcv() [1] failed with errno = %d\n", errno);
	    return PIDSYNC_ERROR;
	}
	for (i = 0; i < numprocs; i++) {
	    if (pids[i] == pidmsg.pid)
		break;
	}
	if (i == numprocs) {
	    /* got a new pid - yeah! */
	    pids[npids] = pidmsg.pid;
	    npids++;

	    pidmsg.cnt++;
	    if (pidmsg.cnt < numprocs) {
		if (msgsnd (qid, (void *)&pidmsg, sizeof(pidsync_msg) - sizeof(long), 0) < 0) {
		    printf ("pidsync: msgsnd() [2] failed with errno = %d\n", errno);
		    return PIDSYNC_ERROR;
		}
	    }
	} else {
	    /* return pid which I allready knew to the queue */
	    if (msgsnd (qid, (void *)&pidmsg, sizeof(pidsync_msg) - sizeof(long), 0) < 0) {
		printf ("pidsync: msgsnd() failed with errno = %d\n", errno);
		return PIDSYNC_ERROR;
	    }
	    usleep (BACKOFF_TIME_US);
	}
    }

    /* leader election and id creation: proc with the smallest pid */
    minpid = pids[0];
    myid = 0;
    for (i = 1; i < numprocs; i++) {
	if (pids[i] < minpid)
	    minpid = pids[i];
	if (pids[i] < mypid)
	    myid++;
    }
    
    /* barrier */
    ackmsg.mtype = myid ? BARRIER_ACKMSG : BARRIER_ZEROMSG;
    ackmsg.id   = myid;
    if (msgsnd (qid, (void *)&ackmsg, sizeof(ack_msg) - sizeof(long), 0) < 0) {
	printf ("pidsync: msgsnd() [3] failed with errno = %d\n", errno);
	return PIDSYNC_ERROR;
    }
    if (myid == 0) {
	/* collect ids */
	for (i = 1; i < numprocs; i++) {
	    if (msgrcv (qid, (void *)&ackmsg, sizeof(ack_msg) - sizeof(long), BARRIER_ACKMSG, 0) < 0) {
		printf ("pidsync: msgrcv() [2] failed with errno = %d\n", errno);
		return PIDSYNC_ERROR;
	    }
	}
	if (msgrcv (qid, (void *)&ackmsg, sizeof(ack_msg) - sizeof(long), BARRIER_ZEROMSG, 0) < 0) {
	    printf ("pidsync: msgrcv() [3] failed with errno = %d\n", errno);
	    return PIDSYNC_ERROR;
	}
    } else {
	if (msgctl (qid, IPC_STAT, &qinfo) == 0) {
	    while (qinfo.msg_qnum) {
		usleep (BACKOFF_TIME_US);
		if (msgctl (qid, IPC_STAT, &qinfo) != 0)
		    break;
	    }
	}
    }

    /* cleanup */
    free (pids);

    return myid;
#endif
}

/*  pidsnd()  */
int pidsnd (int value, int toid, int myid, int qid)
{
    int_msg smsg;

    smsg.mtype  = (long)(INT_MSG + toid);
    smsg.fromid = myid;
    smsg.value  = value;

    if (msgsnd (qid, (void *)&smsg, sizeof(int_msg), 0) < 0) {
	printf ("pidsync: msgsnd() [4] failed with errno = %d\n", errno);
	return PIDSYNC_ERROR;
    }

    return 0;
}

/*  pidrcv()  */
int pidrcv (int fromid, int myid, int qid)
{
    int_msg rmsg;
    
    if (msgrcv (qid, (void *)&rmsg, sizeof(int_msg), INT_MSG + myid, 0) < 0) {
	printf ("pidsync:  msgrcv() [4] failed with errno = %d\n", errno);
	return PIDSYNC_ERROR;
    }

    return (rmsg.value);
}
