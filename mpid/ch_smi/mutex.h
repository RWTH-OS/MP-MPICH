#ifndef MPID_SMI_MUTEX_H
#define MPID_SMI_MUTEX_H


#define MPID_SMI_SSIDE_SYNC_SHRG_SIZE   (128*1024)          /* 128 kB */


int MPID_SMI_Mutex_module_init (void);
void MPID_SMI_Mutex_module_finalize (void);

int MPID_SMI_Mutex_init (int *ID, int num_procs, int * rank_list, int prank);
int MPID_SMI_Mutex_destroy (int ID);

int MPID_SMI_Mutex_lock (int ID);
int MPID_SMI_Mutex_readlock (int ID);
int MPID_SMI_Mutex_unlock (int ID);

#if 0	/* not implemented yet */
int MPID_SMI_Mutex_trylock (int ID, int * result);
#endif
















#endif	/* MPID_SMI_MUTEX_H */

