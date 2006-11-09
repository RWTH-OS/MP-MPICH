/* $Id$ */

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <limits.h>
#ifdef USE_MMAP
#include <sys/mman.h>
#endif

#include "env/smidebug.h"
#include "unix_shmem.h"
#include "utility/general.h"
#include "proc_node_numbers/first_proc_on_node.h"
#include "message_passing/lowlevelmp.h"
#include "shseg_key.h" 

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static key_t shmseg_key = 1111; /* This is a supposed key for a to be created      */
                                /* shared memory segment. The keys are tested      */
                                /* incrementally whether the creation of a segment */
                                /* for this key is possible
				 * */

static char szTempPath[] = "/tmp"; 



/*****************************************************************************/
/*** This functions maps a unix shared segment with the specified id to    ***/
/*** the specified address within the calling processes address space.     ***/
/*** This function can return SMI_ERR_BADADR, if the mapping succeeded,    ***/
/*** but the returned address is not the desired one (the reason for this  ***/
/*** is not known), or SMI_ERR_MAP_FAILED, the the mapping failed totally. ***/
/*** Reasons for this might be, that there is not enough memory, or what   ***/
/*** else.                                                                 ***/
/*****************************************************************************/
smi_error_t _smi_map_unix_shared_segment(shseg_t* shseg)
{
#ifdef WIN32 
#define PATH_MAX MAX_PATH
#endif
    char* return_address;
    /* int i; */
    /* size_t iFullSize; */
#ifdef USE_MMAP
	char szKey[PATH_MAX];
    struct stat sFileStat;
#else
#ifndef WIN32
    struct shmid_ds shm_info;
#endif
#endif

    DSECTION("_smi_map_unix_shared_segment");
    DSECTENTRYPOINT;

#ifdef WIN32 
    /* Mapping a segment with size 0 does really create a zero-sized segment
       under Windows (instead of mapping the complete segment of unknown size 
       as under Unix). Therefore, 0 is not a valid size for Windows. */
    if (shseg->size == 0) {
	DPROBLEM("Can not create a segment of size 0");
	DSECTLEAVE; return(SMI_ERR_NOSEGMENT);
    }
#endif

    DNOTICEI("mapping key",shseg->id);  

#ifdef USE_MMAP
    if (_smi_my_proc_rank != shseg->owner) {
#ifdef HAVE_POSIX_ANON_MMAP
	sprintf(szKey, "/%d.smi.mem", shseg->id);
	shseg->handle = shm_open(szKey, O_RDWR, 0600);
#else
        sprintf(szKey, "%s/%d.smi.mem", szTempPath, shseg->id);
	shseg->handle = open(szKey, O_RDWR, 0600);
	if (shseg->handle == -1) {
	    DNOTICEI("open->errno:", errno);
	}
#endif /* HAVE_POSIX_ANON_MMAP */
	/* Autodetection of size if needed */ 
	if (shseg->size == 0) {
	    fstat(shseg->handle, &sFileStat);
	    shseg->size =  sFileStat.st_size - shseg->offset;
	}
    }
#endif /* USE_MMAP */

    if (shseg->flags & SHREG_NONFIXED) {
        DNOTICE("NONFIXED");
	DNOTICEI("shseg->size",shseg->size);
	DNOTICEI("shseg->offset",shseg->offset);
#ifdef USE_MMAP
	DNOTICEI("shseg->handle",shseg->handle);
	return_address = mmap(0, shseg->size,PROT_READ | PROT_WRITE, MAP_SHARED,
			      shseg->handle, shseg->offset);
	if (return_address == (char *)-1) {
	    DNOTICEI("mmap->errno:", errno);
	}
	shseg->address = return_address;
#else /* no USE_MMAP */
	return_address = shmat(shseg->id, 0, 0600);	       
	DNOTICEP("shmat() returned address", return_address);
	shseg->address = return_address + shseg->offset;

	/* set actual size */
	if (shseg->size == 0) {
#if !defined WIN32 
	    /* This doesnt work under Windows - there is no way to retrieve
	       the size of a shared memory (file) object that a process attaches to. */
	    /* XXX This loop is a semi-workaround for shmem-problems on some Linux 
	       systems. */
	    do {
	    if (shmctl(shseg->id, IPC_STAT, &shm_info) != 0) {
		DWARNINGI ("shmctl(IPC_STAT) failed, errno =", errno);
	    }
	    DNOTICEI("shmctl() gives a segment size of", shm_info.shm_segsz);
	    shseg->size = shm_info.shm_segsz - shseg->offset;
#if 0	    
	    /* XXX debug */
	    fprintf(stderr, "shmctl() for id %d gives a segment size of %d, errno = %d\n", 
		    shseg->id, shm_info.shm_segsz, errno);
#endif
	    } while (shm_info.shm_segsz > 1<<30);
#endif
	}
#endif /* USE_MMAP */
    } else {
	/* this process is the owner of the segment */
        DNOTICE("FIXED");
#ifdef WIN32
	VirtualFree(shseg->address, 0, MEM_RELEASE);
#endif

#ifdef USE_MMAP
	return_address = mmap(shseg->address, shseg->size,
			      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
			      shseg->handle, shseg->offset);
	ASSERT_R(return_address == shseg->address, "Could not get desired adress",SMI_ERR_BADADR);
#else
	return_address = shmat(shseg->id, shseg->address - shseg->offset, 0600);
	ASSERT_R(return_address == shseg->address - shseg->offset, "Could not get desired adress",
		 SMI_ERR_BADADR);
	shseg->address = return_address;
#endif 
    }
    DNOTICEP("SMI address of shared segment is", shseg->address);
    DNOTICEP("effective address of shared segment is", return_address);

    ASSERT_R(return_address != (char*)-1, "Could not attach shared memory", SMI_ERR_MAPFAILED);

    DSECTLEAVE; return(SMI_SUCCESS);
}

  
/*****************************************************************************/
/*** This functions releases the mapping of the specified segment.         ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process.                                                      ***/
/*** A 2xxx error might occure but is not probable.                        ***/
/*****************************************************************************/
smi_error_t _smi_unmap_unix_shared_segment(shseg_t* shseg)
{
    DSECTION("_smi_unmap_unix_shared_segment");
    smi_error_t error; 

    DSECTENTRYPOINT;
    
#ifdef USE_MMAP
    error = munmap(shseg->address, shseg->size);
#else
    error = shmdt(shseg->address - shseg->offset);    
#endif
    ASSERT_R((error!=-1),"Could not detach shared memory",2000+errno);
    
    DSECTLEAVE; return(SMI_SUCCESS);
}
  

/*****************************************************************************/
/*** Creates a Unix shared segments of the specified size on the specified ***/
/*** machine. The resulting identifier is passed back. Therefore, it is at ***/
/*** least necessary that all processes on the specified machine execute   ***/
/*** this function at the same time. For them, this function results in a  ***/
/*** global synchronization.                                               ***/
/*** 1xx errors may occure as well as SMI_ERR_NOSEGMENT in the case that   ***/
/*** no more segment is possible in the system og the function was not able***/
/*** to allocate one for other reasons. One might be that the requested    ***/
/*** segment is too large in size.                                         ***/
/*****************************************************************************/
smi_error_t _smi_create_unix_shared_segment(shseg_t* shseg)
{
   int t, key, i;
   int retry = 1;
   int *buffer;
   smi_error_t mpi_error;
#ifdef USE_MMAP
   char szKey[PATH_MAX];
#endif

   DSECTION("_smi_create_unix_shared_segment");
   DSECTENTRYPOINT;

   t  = _smi_no_processes_on_machine(shseg->machine);
   
   if (_smi_my_machine_rank == shseg->machine) {
       /* determine the rank of the process with the   */
       /* lowest process rank on the specified machine */

       /* The initiator process installes the shared segment */
       /* and, doing so, searches for a free key.            */
       if (_smi_my_proc_rank == shseg->owner) {
	   /* To do so: try several keys as long as the call */
	   /* fails only because the key does already exist. */
	   do {
	       /* create new key */
	       shmseg_key++;
	       key = (t+1)*shmseg_key + t;
	       key = _smi_modify_key(key);
	       DNOTICEI("Trying to create local shared memory with key", key);
#ifdef USE_MMAP
#ifdef HAVE_POSIX_ANON_MMAP
	       sprintf(szKey, "/%d.smi.mem", key);
	       shseg->handle = shm_open(szKey, O_CREAT | O_EXCL | O_RDWR, 0600); 
	       retry = (shseg->handle == -1) && (errno == EEXIST);
#else
	       sprintf(szKey, "%s/%d.smi.mem", szTempPath, key);
	       shseg->handle = rs_CreateTempfile(szKey, O_CREAT | O_EXCL | O_RDWR);
	       retry = (shseg->handle == -1) && (errno == EEXIST);
#endif /* HAVE_POSIX_ANON_MMAP */
#else       
	       shseg->id = rs_shmget(key, shseg->size, IPC_CREAT|IPC_EXCL|0600);
	       retry = (shseg->id == -1) && (errno == EEXIST);;
#endif /* USE_MMAP */
	   } while (retry && (key < (1 << LD_MAX_SHSEG_KEY)));

#ifdef USE_MMAP
	   if (shseg->handle == -1) {
	       DNOTICEI("shseg->handle", shseg->handle);
	       DNOTICEI("errno:" ,errno);
	       key = -1;
	   }
	   else {
	       shseg->id = key;
	   
	       DNOTICE("sizing shared memory file");
	       ftruncate(shseg->handle, shseg->size); 
	   }
	   /* XXX error check here! */
#endif
       }
       
       if (!(shseg->flags & SHREG_ASYNC)) {
	   DNOTICE("collective mapping of memory on all processes");
	   /* The identifier is passed to all processes within this              */
	   /* machine. This is also used to check if the shmget() was successful */
	   /* on all machines.                                                   */
	   ALLOCATE(buffer, int *, _smi_nbr_procs * sizeof(int));
	   mpi_error = _smi_ll_allgather((int *)&(shseg->id), 1, buffer,
					 _smi_my_proc_rank);
	   ASSERT_R((mpi_error == MPI_SUCCESS),"MPIAllgather failed",1000+mpi_error);
	 
	   for (i = 0; i < _smi_nbr_machines; i++) {
	       if (buffer[_smi_first_proc_on_node(i)] == -1) {
		 /* segment creation has failed at least on one node - */
		 /*   test if the local segment needs to be removed    */
		   if (buffer[_smi_first_proc_on_node(shseg->machine)] != -1) {
		       DNOTICE ("removing local segment");
		       _smi_remove_unix_shared_segment(shseg);
		   }
		   DPROBLEM ("shmget failed");
		   DSECTLEAVE; return (SMI_ERR_NOMEM);
	       }
	   }
	   shseg->id = buffer[shseg->owner];
	   free(buffer);  
       }
   }

   /*
     #ifdef WIN32
     if (my_proc_rank != shseg->owner)
     rs_shmget(key, shseg->size, 0600);
     #endif
   */

   DSECTLEAVE;
   return(SMI_SUCCESS);
}
   
   

/*****************************************************************************/
/*** Removes a Unix shared segment with the specified identifier.          ***/
/*** A 2xxx error is possible but not probable.                            ***/
/*****************************************************************************/
smi_error_t _smi_remove_unix_shared_segment(shseg_t* shseg)
{
    DSECTION("_smi_remove_unix_shared_segment");
    smi_error_t error = 0; 
#ifdef USE_MMAP
    char szKey[PATH_MAX];
#endif 

    DSECTENTRYPOINT;

#ifndef WIN32
    if (shseg->owner == _smi_my_proc_rank) {
#endif
#ifdef USE_MMAP
	error = close(shseg->handle);
	if (_smi_my_proc_rank == shseg->owner) {
#ifdef HAVE_POSIX_ANON_MMAP
	    sprintf(szKey, "/%d.smi.mem", shseg->id);
	    shm_unlink(szKey);
#else
	    sprintf(szKey, "%s/%d.smi.mem", szTempPath, shseg->id);
	    rs_RemoveTempfile(szKey);
#endif
	}
#else
	error = rs_shmctl(shseg->id, IPC_RMID, 0);
#endif
#ifndef WIN32
    }
#endif   
    ASSERT_R((error!=-1),"Could not remove shared memory",2000+errno);

    DSECTLEAVE;
    return(SMI_SUCCESS);
 }








