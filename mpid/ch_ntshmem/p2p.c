#define P2P_EXTERN

/* This is the main file for the p2 shared-memory code (from P4, hence p2)
   
   In order to keep this file from getting too ugly, I've broken it down
   into subfiles for the various options.  

   The subfiles are:
   
   p2pshmat - SYSV shared memory allocator
   p2pshop  - SYSV semaphores
   p2pmmap  - Memory map
   p2pcnx   - Code for the Convex SPP
   p2pirix  - Code for special IRIX routines (usmalloc etc).
   p2pwinprocs - Windows code for process creation

   These are INCLUDED in this file so that the Makefile doesn't need to
   know which files to compile.
 */

/*SILKE: Achtung, hier wird malloc und calloc neu definiert!!!!!*/
#include "mpid.h"
#ifdef malloc
#undef malloc
#undef free
#undef calloc
#endif



#ifndef DBG
 #ifdef _DEBUG
 #include <stdio.h>
 #define DBG(m) printf("%s\n",m);
 #else
 #define DBG(m)
 #endif
#endif

#include "mpiddev.h"

#include "mpid_debug.h"
#include "p2p.h"
#include <stdio.h>


#define p2p_dprintf printf

void *xx_shmalloc ANSI_ARGS((unsigned));
void xx_shfree ANSI_ARGS((char *));
void xx_init_shmalloc ANSI_ARGS(( char *, unsigned ));
void p2p_syserror ANSI_ARGS(( char *, int ));

SECURITY_ATTRIBUTES attr = {sizeof(SECURITY_ATTRIBUTES),0,TRUE};

static p2p_lock_t *p2p_shmem_lock;	/* Pointer to lock */

#ifdef MPID_CACHE_LINE_SIZE
#define ALIGNMENT (2*MPID_CACHE_LINE_SIZE)
#define LOG_ALIGN (MPID_CACHE_LINE_LOG_SIZE+1)
#else
#define LOG_ALIGN 6
#define ALIGNMENT (1 << LOG_ALIGN)
#endif
/* ALIGNMENT is assumed below to be bigger than sizeof(p2p_lock_t) +
   sizeof(Header *), so do not reduce LOG_ALIGN below 4 */

union header
{
    struct
    {
	union header *ptr;	/* next block if on free list */
	unsigned size;		/* size of this block */
    } s;
    char align[ALIGNMENT];	/* Align to ALIGNMENT byte boundary */
};

typedef union header Header;

static Header **freep;		/* pointer to pointer to start of free list
                                   *freep = NULL: shared memory is entirely
used */
 /* Here is the WIN32 code */
HANDLE  MemHandle=NULL;

void *actual_mapping_address;
unsigned int mem_size;
void p2p_init(maxprocs,memsize)
int maxprocs;
int memsize;
{	
	char Handle[12];
	if(!GetEnvironmentVariable("MPICH_SHMEM_HANDLE",Handle,12)) {
		MemHandle=CreateFileMapping(
			(HANDLE)INVALID_HANDLE_VALUE,&attr,PAGE_READWRITE,0,memsize,NULL);
		if(!MemHandle) {
			p2p_syserror( "Cannot create file mapping", GetLastError() );
		}
		actual_mapping_address=MapViewOfFile(MemHandle,FILE_MAP_ALL_ACCESS,0,0,0);
		if(!actual_mapping_address) {
			p2p_syserror("Cannot map view of file",GetLastError());
		}
		*(int*)actual_mapping_address=1;
	} else {
#ifdef _WIN64
		MemHandle=(void*)_atoi64(Handle);
#else
		MemHandle=(void*)atoi(Handle);
#endif
		actual_mapping_address=MapViewOfFile(MemHandle,FILE_MAP_ALL_ACCESS,0,0,0);
		if(!actual_mapping_address) {
			p2p_syserror("Cannot map view of file",GetLastError());
		}
	}
	mem_size=memsize;
	
	/* Maybe we have to remap the FileMapping object later, when all processes
	   have been created so we can negotiate the mapping address */
}

#include "p2pwinprocs.c"

void p2p_syserror( string, value )
char *string;
int  value;
{
	static char all[1024];
	void *lpMsgBuf;
	strcpy(all,string);
	strcat(all,":\n");
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,value,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language*/
				(LPTSTR) &lpMsgBuf,0,NULL);
	
	strcat(all,(char*)lpMsgBuf);
	p2p_error(all,value);
}



void p2p_error(string,value)
char * string;
int value;
{
    fprintf(stderr,"%s (%d)\n",string, value); fflush(stderr);
	p2p_kill_procs();
	p2p_cleanup();

	exit(99);

}

LARGE_INTEGER Frequency;
void p2p_wtime_init()
{
	QueryPerformanceFrequency(&Frequency);
}

double p2p_wtime()
{
	/* returns seconds */
	LARGE_INTEGER Counter;
	QueryPerformanceCounter(&Counter);
	return (double)Counter.QuadPart / (double)Frequency.QuadPart;
}

void p2p_yield()
{
	Sleep(0);
}



#if defined(USE_XX_SHMALLOC)
/* This is not machine dependent code but is only used on some machines */

/*
  Memory management routines from ANSI K&R C, modified to manage
  a single block of shared memory.
  Have stripped out all the usage monitoring to keep it simple.

  To initialize a piece of shared memory:
    xx_init_shmalloc(char *memory, unsigned nbytes)

  Then call xx_shmalloc() and xx_shfree() as usual.
*/


void xx_init_shmalloc(memory, nbytes)
char *memory;
unsigned nbytes;
/*
  memory points to a region of shared memory nbytes long.
  initialize the data structures needed to manage this memory
*/
{
	
    int nunits = nbytes >> LOG_ALIGN;	
    Header *region = (Header *) memory;

    /* Quick check that things are OK */
#ifdef SHM_DEBUG
	DBG("xx_init_shmalloc");
#endif

    if (ALIGNMENT != sizeof(Header) ||
	ALIGNMENT < (sizeof(Header *) + sizeof(p2p_lock_t)))
    {
        p2p_dprintf("%d %d\n",sizeof(Header),sizeof(p2p_lock_t));
	p2p_error("xx_init_shmem: Alignment is wrong", ALIGNMENT);
    }

    if (!region)
	p2p_error("xx_init_shmem: Passed null pointer", 0);

    if (nunits < 2)
	p2p_error("xx_init_shmem: Initial region is ridiculously small",
		 (int) nbytes);

    /*
     * Shared memory region is structured as follows
     *
     * 1) (Header *) freep ... free list pointer 2) (p2p_lock_t) p2p_shmem_lock
...
     * space to hold lock 3) padding up to alignment boundary 4) First header
     * of free list
     */

    freep = (Header **) region;	/* Free space pointer in first block  */

	p2p_shmem_lock=&tmp_mutex;
    (region + 1)->s.ptr = *freep = region + 1;	/* Data in rest */
    (region + 1)->s.size = nunits - 1;	/* One header consumed already */



}

void *xx_shmalloc(nbytes)
unsigned nbytes;
{

    Header *p, *prevp;
    char *address = (char *) NULL;
    unsigned nunits;
#ifdef SHM_DEBUG
	DBG("xx_shmalloc");
#endif
    /* Force entire routine to be single threaded */
    (void)p2p_lock(p2p_shmem_lock);


    if (*freep) {
        /* Look for free shared memory */

    nunits = ((nbytes + sizeof(Header) - 1) >> LOG_ALIGN) + 1;

    prevp = *freep;
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr)
    {
	if (p->s.size >= nunits)
	{			/* Big enuf */
	    if (p->s.size == nunits)	/* exact fit */
            {
           	if (p == p->s.ptr)
                {
                   /* No more shared memory available */
                   prevp = (Header *) NULL;
              	}
               	else {
	  	   prevp->s.ptr = p->s.ptr;
             	}
            }
	    else
	    {			/* allocate tail end */
		p->s.size -= nunits;
		p += p->s.size;
		p->s.size = nunits;
	    }
	    *freep = prevp;
	    address = (char *) (p + 1);
	    break;
	}
	if (p == *freep)
	{			/* wrapped around the free list ... no fit
				 * found */
	    address = (char *) NULL;
	    break;
	}
    }
    }

    /* End critical region */
    (void) p2p_unlock(p2p_shmem_lock);

	    /*
    if (address == NULL)
	p2p_dprintf("xx_shmalloc: returning NULL; requested %d
bytes\n",nbytes);
	*/
    return address;
}

void xx_shfree(ap)
char *ap;
{
    Header *bp, *p;

    if (!ap)
	return;			/* Do nothing with NULL pointers */

    /* Begin critical region */

    (void) p2p_lock(p2p_shmem_lock);

    bp = (Header *) ap - 1;	/* Point to block header */

    if (*freep) {
         /* there are already free region(s) in the shared memory region */

    	for (p = *freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
	if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
	    break;		/* Freed block at start of end of arena */

    	}

        /* Integrate bp in list */

    	*freep = p;

    if (bp + bp->s.size == p->s.ptr)
    {				/* join to upper neighbour */
                if (p->s.ptr == *freep) *freep = bp;
                if (p->s.ptr == p) bp->s.ptr = bp;
                else               bp->s.ptr = p->s.ptr->s.ptr;

	bp->s.size += p->s.ptr->s.size;
    }
    else
	bp->s.ptr = p->s.ptr;

    if (p + p->s.size == bp)
    {				/* Join to lower neighbour */
	p->s.size += bp->s.size;
	p->s.ptr = bp->s.ptr;
    }
    else
	p->s.ptr = bp;

    }
    else {
        /* There wasn't a free shared memory region before */

       	bp->s.ptr = bp;

       	*freep = bp;
    }

    /* End critical region */
    (void) p2p_unlock(p2p_shmem_lock);
}


#endif

