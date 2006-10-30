/* $Id$ */

/*****************************************************************************/
/* These Algorithms are taken from the SMI lib and modified to our need      */
/*****************************************************************************/

#include <stdlib.h>
#include <unistd.h>


#include "smi.h"

#include "mpiimpl.h"
#include "mpid.h"
#include "sbcnst2.h"
#include "smidef.h"
#include "mmu.h"
#include "mutex.h"
#include "smidev.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* This structure stores all information relevant for mutexes */
typedef struct {
    volatile int	* shmaddr;
    int 			home_of_data, 
					high;
	int				num_procs;
	int				* ranks;
} mutex_id;

#define MAX_BACKOFF 1000000
#define BACKOFF(spin) { spin *= 2; usleep(spin); if (spin >= MAX_BACKOFF) spin = 1; if (spin == 0) spin = 1; }

static int MPID_SMI_Lamport_read_unlock (mutex_id *);
static int MPID_SMI_Lamport_unlock(mutex_id *);
static int MPID_SMI_Lamport_init (mutex_id *ID);
static int MPID_SMI_Lamport_destroy(mutex_id *ID);
static int MPID_SMI_Lamport_lock(mutex_id *ID);
static int MPID_SMI_Lamport_readlock(mutex_id *ID);

/* The variable all_mutex allotes a number to a mutex-identifier. 
	A mutex-identifier contains all necessary informations to use a mutex- 
	algorithm. */
static mutex_id **all_mutex = NULL;

/* This variable stores the size of the array all_mutex */
static int mutex_array_size = 0;

/* This variable counts the mutexs, which are initialized. */
static int mutex_counter = 0;

/* This variable points to a slot that is probably free, it's used to
	speed up the process */
static int last_mutex = 0;

/* For thread safety we do need a pthread mutex */
#ifdef MPID_USE_DEVTHREADS
static pthread_mutex_t pmutex;
#endif


int MPID_SMI_Mutex_module_init (void)
{
	MPID_SMI_INIT_LOCK (&pmutex);

	mutex_counter = 0;
	all_mutex = NULL;
	last_mutex = 0;
	mutex_array_size = 0;

	return 1;
}


void MPID_SMI_Mutex_module_finalize (void)
{
	int	i;

	if (mutex_counter > 0) {
		for (i=0; i<mutex_array_size; i++) {
			if (all_mutex[i])
				MPID_SMI_Mutex_destroy (i);
		}
	}
	if (all_mutex != NULL)
		FREE (all_mutex);

	mutex_counter = 0;
	all_mutex = NULL;
	last_mutex = 0;
	mutex_array_size = 0;

	MPID_SMI_DESTROY_LOCK (&pmutex);
}


/*****************************************************************************/
/* This function updates the array all_mutex. After that it calls the        */
/* initialize-function from the mutex-algorithm. All users of the            */
/* SMI-Library must call this function to intialize the mutex-algorithm.     */
/* The third parameter specifies, at which process, the mutex' data          */
/* structures are to be allocated. If it is '-1', they shall be distribued   */
/* evenly within the whole system.                                           */
/*****************************************************************************/   
int MPID_SMI_Mutex_init (ID, num_procs, rank_list, prank)
	int 		*ID;
	int			num_procs;
	int			* rank_list;
	int 		prank;
{
	int			error;
	int 		i;
	mutex_id	**new_array, 
				*new_mutex;
	int			* new_ranklist;

	MPID_SMI_LOCK (&pmutex);

	if (num_procs <= 0 || !rank_list || prank < 0 || prank >= num_procs)
		return 0;  

	if (!all_mutex || mutex_counter >= mutex_array_size) {
		if (!all_mutex) {
			mutex_array_size = 256;
			last_mutex = 0;
			mutex_counter = 0;
		} else {
			last_mutex = mutex_array_size;
			mutex_array_size *= 2;
		}
		new_array = realloc (all_mutex, mutex_array_size * sizeof(mutex_id*));
		if (!new_array)
			return 0;
		all_mutex = new_array;
	
		/* init array */
		for (i = last_mutex; i < mutex_array_size; i++)
			all_mutex[i] = NULL;
	}
	/* search free slot */
	for (i = 0; i < mutex_array_size; i++) 
		if (!all_mutex[(last_mutex + i) % mutex_array_size]) 
			break;
	last_mutex = (last_mutex + i) % mutex_array_size;


	new_mutex = (mutex_id *) MALLOC (sizeof (mutex_id));
	if (!new_mutex) {
		return 0;
	}
	new_ranklist = MALLOC (sizeof (int) * num_procs);
	if (!new_ranklist) {
		FREE (new_mutex);
		return 0;
	}
	all_mutex[last_mutex] = new_mutex;
	mutex_counter++;
	*ID = last_mutex;
	last_mutex++;	/* for speed up */
  
	new_mutex->shmaddr = NULL;
	new_mutex->home_of_data = prank;
	new_mutex->num_procs = num_procs;
	memcpy (new_ranklist, rank_list, sizeof (int) * num_procs);
	new_mutex->ranks = new_ranklist;

	MPID_SMI_UNLOCK (&pmutex);

	/* Using Lamport's algorithm */
	if (!MPID_SMI_Lamport_init(new_mutex))
		return 0;

#if 0
	SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));
	SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
#endif
  
	return 1;
}


/*****************************************************************************/
/* This function calls the destroy-function from the mutex-algorithm.        */
/* After that it updates the array all_mutex. 					             */
/*****************************************************************************/
int MPID_SMI_Mutex_destroy (ID)
	int	ID;
{
	if (!(ID >= 0 && ID < mutex_counter && all_mutex[ID] != NULL)) {
		return 0;
	}

	if (!MPID_SMI_Lamport_destroy(all_mutex[ID])) {
		return 0;
	} 

	MPID_SMI_LOCK (&pmutex);
 
	FREE (all_mutex[ID]->ranks);
	FREE (all_mutex[ID]);
	all_mutex[ID] = NULL;
	mutex_counter--;

	MPID_SMI_UNLOCK (&pmutex);
  
	return 1;
}


/*****************************************************************************/
/* This function calls the lock-function from the mutex-algorithm.           */
/*****************************************************************************/
int MPID_SMI_Mutex_lock (ID)
	int	ID;
{
	if (!(ID >= 0 && ID < mutex_counter && all_mutex[ID] != NULL))
		return 0;
 
	if (all_mutex[ID]->num_procs == 1)
		return 1;

	if (!MPID_SMI_Lamport_lock(all_mutex[ID]))
		return 0;
 
	return 1;
}


int MPID_SMI_Mutex_readlock (ID)
	int	ID;
{
	if (!(ID >=0 && ID < mutex_counter && all_mutex[ID] != NULL)) 
		return 0;
 
	if (all_mutex[ID]->num_procs == 1)
		return 1;

	if (!MPID_SMI_Lamport_readlock(all_mutex[ID]))
		return 0;
 
	return 1;
}


/*****************************************************************************/
/* This function calls the lock-function from the mutex-algorithm. It it     */
/* gets the loack, 'result is set to '1', otherwise, result ist set to '0'.  */
/*****************************************************************************/
#if 0	/* not yet implemented */
int MPID_SMI_Mutex_trylock (ID, result)
	int	ID;
	int	* result;
{
	int error;
  
	if (!((ID>=0)&&(ID<mutex_counter)&&(all_mutex[ID]!=NULL))) 
		return 0;
 
	if (all_mutex[ID].num_procs == 1)
		return 1;
  
	error = MPID_SMI_Lamport_trylock(all_mutex[ID], result);

	SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
	/* _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1); */
  
	return error;
}
#endif 


/*****************************************************************************/
/* This function calls the unlock-function from the mutex-algorithm.         */
/*****************************************************************************/
int MPID_SMI_Mutex_unlock (ID)
	int ID;
{
	if (!(ID >= 0 && ID < mutex_counter && all_mutex[ID] != NULL)) 
		return 0;
 
	if (all_mutex[ID]->num_procs == 1)
		return 1;

	SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));
	/* _smi_range_store_barrier(NULL, ALLSTREAMS_SIZE, -1); */

	if (!MPID_SMI_Lamport_unlock(all_mutex[ID]))
		return 0;
  
	return 1;
}

/*****************************************************************************/
/* This functions discribe the mutex algorithm from Leslie Lamport.          */
/* When the system has no contention, a process enters the critical section  */
/* by O(1) statements. In worst case the process has to visit N variables.   */
/*****************************************************************************/ 
static int MPID_SMI_Lamport_init (mutex_id *ID)
{
	int 	error;
	int 	i;
	struct _sendinfo {
		size_t	offset;
		int		id;
	} 		sendinfo;

	if (ID->home_of_data == -1)
		ID->home_of_data = ID->ranks [0];

	if (MPID_SMI_myid == ID->home_of_data) {
		ID->shmaddr = MPID_SMI_Alloc_mem_internal ((ID->num_procs+3) * sizeof(int), 
													MUST_BE_SHARED, NO_ALIGNMENT);
		if (!ID->shmaddr)
			return 0;

		/* Initialize variables */
		ID->shmaddr[0] = ID->shmaddr[1] = -1;
		ID->shmaddr[2] = 0;
		for(i = 3; i < ID->num_procs+3; i++)
			ID->shmaddr[i] = 0;

		/* broadcast addr of shmaddr */
		if (!MPID_SMI_Addr_to_offset ((void *)ID->shmaddr, &sendinfo.id, &sendinfo.offset))
			return 0;
		for (i = 0;  i < ID->num_procs; i++) {
			if (ID->ranks[i] == MPID_SMI_myid)
				continue;
			SMIcall (SMI_Send (&sendinfo, sizeof (struct _sendinfo), ID->ranks[i]));
		}
	} else {
		SMIcall (SMI_Recv (&sendinfo, sizeof(struct _sendinfo), ID->home_of_data));
		if (!MPID_SMI_Shreg_tryconnect (ID->home_of_data, sendinfo.id))
			return 0;
		ID->shmaddr = MPID_SMI_Offset_to_addr (ID->home_of_data, sendinfo.id, sendinfo.offset);
		if (!ID->shmaddr) return 0;
	}
	return 1;
}


static int MPID_SMI_Lamport_destroy(mutex_id *ID)
{
	if (ID->shmaddr == NULL)
		return 0;

	if (ID->home_of_data == MPID_SMI_myid) {
		MPID_SMI_Free_mem ((void *)ID->shmaddr);
	} else {
		MPID_SMI_Shreg_disconnect (MPID_SMI_Get_shreg ((void *)ID->shmaddr));
	}
	return 1;
}


static int MPID_SMI_Lamport_lock(mutex_id *ID)
{
	volatile int *x, *y;
	volatile int *request;
	volatile int *refcount;
	int j, spin;
  
	MPID_STAT_ENTRY(ch_smi_mutex_lock);

	x = &ID->shmaddr[0];
	y = &ID->shmaddr[1];
	refcount = &ID->shmaddr[2];
	request = &ID->shmaddr[3];

	/* in the comments, RW means 'remote write' while RR stands for 'remote read' */
	while (1) {
		SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
		/* spinning on refcount - if anyone has read-access-lock, we can not
		   get a write lock. */
		spin = 0;
		while (*refcount > 0 /* RR */)
			BACKOFF(spin)
	
		/* getting exclusive lock */
		while (1) {  
			for (request[MPID_SMI_myid] = 1 /* RW */, *x = MPID_SMI_myid /* RW */; 
				 *y != -1;  /* RR */
				 request[MPID_SMI_myid] = 1 /* RW */, *x = MPID_SMI_myid /* RW */) {
				request[MPID_SMI_myid] = 0; /* RW */
				spin = 0;
				while (*y != -1 /* RR */) 
					BACKOFF(spin)
			}
			*y = MPID_SMI_myid; /* RW */
			SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));

			if (*x != MPID_SMI_myid /* RR */) {
				request[MPID_SMI_myid] = 0; /* RW */
				for (spin = 0, j = 0; j < ID->num_procs; j++)
					while (request[j] != 0  /* RR */) 
						BACKOFF(spin)
				if (*y != MPID_SMI_myid  /* RR */) {
					spin = 0;
					while (*y != -1 /* RR */) 
						BACKOFF(spin)						
				} else {
					break;
				}
			} else {
				break;
			}
		}
		SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
		if (*refcount == 0 /* RR */) {
			break;
		} else {
			/* releasing exclusive lock to spin on refcount again */
			*y = -1; /* RW */
			request [MPID_SMI_myid] = 0; /* RW */
		}
	}

	MPID_STAT_EXIT(ch_smi_mutex_lock);
	return 1;
}


static int MPID_SMI_Lamport_readlock(mutex_id *ID)
{
	volatile int *x, *y;
	volatile int *request;
	volatile int *refcount;
	int j;
  
	MPID_STAT_ENTRY(ch_smi_mutex_lock);

	x = &ID->shmaddr[0];
	y = &ID->shmaddr[1];
	refcount = &ID->shmaddr[2];
	request = &ID->shmaddr[3];

	/* getting exclusive lock */
	/* in the comments, RW means 'remote write' while RR stands for 'remote read' */
	while(1) {  
		SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));

		for (request[MPID_SMI_myid] = 1 /* RW */, *x = MPID_SMI_myid; /* RW */
			*y != -1;  /* RR */
			request[MPID_SMI_myid] = 1 /* RW */, *x = MPID_SMI_myid /* RW */) {
			request[MPID_SMI_myid] = 0; /* RW */
			while (*y != -1 /* RR */) 
				/* Spinning - XXX: back off! */
				;
		}
		*y = MPID_SMI_myid; /* RW */
		SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));
		if (*x != MPID_SMI_myid /* RR */) {
			request[MPID_SMI_myid] = 0; /* RW */
			for (j = 0; j < ID->num_procs; j++)
				while (request[j] != 0 /* RR */)
					/* Spinning - XXX: back off! */
					;
			if (*y != MPID_SMI_myid /* RR */) {
				while (*y != -1 /* RR */) 
					/* Spinning - XXX: back off! */
					;
			} else {
				break;
			}
		} else {
			break;
		}
	}

	/* increment refcount */
	SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
	*refcount = *refcount + 1; /* RR + RW */
	SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));

	/* release exclusive lock */
	*y = -1; /* RW */
	request [MPID_SMI_myid] = 0; /* RW */

	MPID_STAT_EXIT(ch_smi_mutex_lock);
	return 1;
}


#if 0	/* doesn't work yet */
int MPID_SMI_Lamport_trylock(ID, result)
	mutex_id	* ID;
	int			* result;
{
	volatile int *x, *y;
	volatile int *request;
	int j;
  
	if (ID == NULL || ID->shmaddr == NULL || result == NULL)
		return 0;

	x = &ID->shmaddr[0];
	y = &ID->shmaddr[1];
	request = &ID->shmaddr[2];

	request [MPID_SMI_myid] = 1;
	*x = MPID_SMI_myid;

	if (*y == -1) {
		*y = MPID_SMI_myid;
		SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));

		if (*x == MPID_SMI_myid) {
			*result = 1;
		} else {
			request [MPID_SMI_myid] = 0;
			*result = 0;
			/* what to do here ?... */
		}
	} else {
		request [MPID_SMI_myid] = 0;
		*result = 0;
	}

	return 1;
}
#endif


static int MPID_SMI_Lamport_unlock(ID)
	mutex_id	*ID;
{
	MPID_STAT_ENTRY(ch_smi_mutex_unlock);

	if (ID->shmaddr[2] > 0 /* RR */) {
		MPID_STAT_EXIT(ch_smi_mutex_unlock);
		/* this is a read-lock */
		return MPID_SMI_Lamport_read_unlock(ID);
	}

	/* else release exclusive lock */
	ID->shmaddr [1] = -1; /* RW */
	ID->shmaddr [MPID_SMI_myid+3] = 0; /* RW */
	SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));
   
	MPID_STAT_EXIT(ch_smi_mutex_unlock);
	return 1;
}


static int MPID_SMI_Lamport_read_unlock(ID)
	mutex_id	*ID;
{
	volatile int *x, *y;
	volatile int *request;
	volatile int *refcount;
	int j;
  
	MPID_STAT_ENTRY(ch_smi_mutex_unlock);

	x = &ID->shmaddr[0];
	y = &ID->shmaddr[1];
	refcount = &ID->shmaddr[2];
	request = &ID->shmaddr[3];

	/* getting exclusive lock */
	while(1) {  
		SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
		
		for (request[MPID_SMI_myid] = 1 /* RW */, *x = MPID_SMI_myid /* RW */; 
			 *y != -1; /* RR */ 
			 request[MPID_SMI_myid] = 1  /* RW */, *x = MPID_SMI_myid /* RW */) {
			request[MPID_SMI_myid] = 0; /* RW */
			while (*y != -1 /* RR */) 
				; /* Spinning - XXX backoff! */
		}
		*y = MPID_SMI_myid; /* RW */
		SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));

		if (*x != MPID_SMI_myid /* RR */) {
			request[MPID_SMI_myid] = 0; /* RW */
			for (j = 0; j < ID->num_procs; j++)
				while (request[j] != 0 /* RR */) 
					; /* XXX Spinning */
			if (*y != MPID_SMI_myid /* RR */) {
				while (*y != -1 /* RR */) 
					; /* Spinning */
			} else {
				break;
			}
		} else {
			break;
		}
	}

	/* decrement refcount */
	SMIcall (SMI_Flush_read (SMI_FLUSH_ALL));
	*refcount = *refcount - 1;  /* RR + RW */
	SMIcall (SMI_Flush_write (SMI_FLUSH_ALL));

	/* releasing exclusive lock */
	*y = -1; /* RW */
	request [MPID_SMI_myid] = 0; /* RW */

	MPID_STAT_EXIT(ch_smi_mutex_unlock);
	return 1;
}


int MPID_SMI_Location_of_mutex (mutexid, location)
	int	mutexid;
	int	* location;
{
	if (!(mutexid >= 0 && mutexid < mutex_array_size &&
								all_mutex[mutexid] != NULL) || !location)
		return 0;

	*location = all_mutex[mutexid]->home_of_data;

	return 1;
}













/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
