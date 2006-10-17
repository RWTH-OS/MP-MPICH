/* $Id$ */

#ifndef _46432_SHMEMSYSV_H
#define _46432_SHMEMSYSV_H

#include "shmem-mpid.h"

/* maximum number of semaphores in our semaphore set */
#define MPID_SHMEM_SYSV_MAX_SEMS 25 

/* size of shared memory segments that we try to get if we cannot
   get the whole shared memory as a single segment */
#define MPID_SHMEM_SYSV_SHM_SEGSIZE (1*1024*1024)

#ifdef PROCESSOR_COUNT

#if PROCESSOR_COUNT > 32
#define MPID_MAX_PROCS PROCESSOR_COUNT
#define MPID_MAX_SHMEM 4194304*(PROCESSOR_COUNT/8)
#else
#define MPID_MAX_PROCS 32
#define MPID_MAX_SHMEM 4194304
#endif /* PROCESSOR_COUNT > 32 */

#else

#define MPID_MAX_PROCS 32
#define MPID_MAX_SHMEM 4194304

#endif /* PROCESSOR_COUNT */

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

typedef struct {
  int semnum;
}   p2p_lock_t;

#define p2p_lock_name     "semop_lock"

struct _MPID_SHMEM_sem_admin_t {
    int sysv_next_lock;
    p2p_lock_t slave_lock;
};

typedef struct _MPID_SHMEM_sem_admin_t MPID_SHMEM_sem_admin_t;


extern int MPID_SHMEM_sysv_semid;
extern int MPID_SHMEM_sysv_pid[MPID_MAX_PROCS];
extern int MPID_SHMEM_sysv_mypid;
extern MPID_SHMEM_sem_admin_t *MPID_SHMEM_semaphore_admin;
extern p2p_lock_t *MPID_SHMEM_sysv_firstlock; /* Pointer to lock */


/* function prototypes */

void *MPID_SHMEM_sysv_create_shmem( unsigned int );
void MPID_SHMEM_sysv_shmem_cleanup( void );
int MPID_SHMEM_sysv_init_semset( int, int );
void MPID_SHMEM_sysv_lock( p2p_lock_t * );
void MPID_SHMEM_sysv_unlock( p2p_lock_t * );
int MPID_SHMEM_sysv_connect_to_clients( void );
int MPID_SHMEM_sysv_connect_to_master( int );
void *MPID_SHMEM_sysv_malloc( unsigned int );
void MPID_SHMEM_sysv_free( void * );

#endif /* _46432_SHMEMSYSV_H */
