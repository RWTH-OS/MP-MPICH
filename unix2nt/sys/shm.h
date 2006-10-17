/* $Id: shm.h,v 1.1 2001/11/08 23:05:52 martin Exp $ */

#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#ifdef __cplusplus
extern "C" {
#endif

#define O_RDWR  0 
/* #define O_WRONLY 1 */
#define O_RDONLY 2

typedef int key_t;
#define IPC_PRIVATE ((key_t) 0)
#define IPC_RMID 0              /* remove resource */
#define IPC_STAT 2              /* get ipc_perm options */
#define IPC_CREAT 00001000      /* create if key is nonexistent */
#define IPC_EXCL  00002000      /* fail if key exists */

#define SHM_RDONLY 010000       /* read-only access */

struct shmid_ds { 
	int shm_segsz;              /* size of segment (bytes) */	
};

int shmctl (int shmid, int cmd, struct shmid_ds *buf);
int shmget (key_t key, int size, int shmflg);
void *shmat(int shmid, void *shmaddr, int shmflg);
int shmdt (void *shmaddr);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SHM_H */
