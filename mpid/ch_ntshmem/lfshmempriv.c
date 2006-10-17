/*
   $Id$

   This file contains routines that are private and unique to the ch_ntshmem
   implementation
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "mpid_debug.h"
#include "statistic.h"
#include "shdef.h"
#include "shpackets.h"

#include "p2p.h"

#ifdef _WIN32
#include <cmnargs.h>
#endif

#include "LogMpid.h"

#ifndef DBG
#ifdef _DEBUG
#include <stdio.h>
#define DBG(m) printf("%s\n",m);
#else
#define DBG(m)
#endif
#endif

/* MPID_PKT_TSH -> MPID_PKT_TSH whole file */
#if 0
__declspec(naked) 
void mmx_memcpy(char *dst, char *src, int n) {
    _asm {
	push esi;
	push edi;
	
	mov edi, DWORD PTR [ESP+12];
	mov esi, DWORD PTR [ESP+16];
	mov eax, [ESP+20];
	
	;test edi,111b ; /*Align destination to 8 byte boudary*/
	;jz lines ;
	;mov ecx,edi;
	;and ecx,111b;
	;sub eax,ecx ;
	;rep movsb;
	
lines:		mov ecx,eax; 
		shr ecx,5;  /* Divide #bytes by 32 */
		jz dwords ;
		
align 4			
stream:	        movq mm0, qword ptr [esi] ; 
		movq mm1, qword ptr [esi+8] ;
		movq mm2, qword ptr [esi+16] ;
		movq mm3, qword ptr [esi+24] ;
		
		
		movq qword ptr [edi]    ,mm0 ;
		movq qword ptr [edi+8]  ,mm1 ;
		movq qword ptr [edi+16] ,mm2 ;
		movq qword ptr [edi+24] ,mm3;
		
                
		add esi,32;
                add edi,32;
		dec ecx
                jnz stream;
		
		;and eax,0x1F; 
                and eax,31; /* n%=64*/
                ;jz end;    /* nothing remaining */
		
dwords:         mov ecx,eax;
                shr ecx,2; /* divide the remainig bytes by 4*/
                ;jz bytes;  /* n < 4 */
                rep movsd;
		
bytes:          and eax,3; /* n %= 4 */
                ;jz end;
                mov ecx,eax;
                rep movsb;
end:
		emms ;
		pop edi;
		pop esi;
		ret;
    }
}

#endif



/* Pointer used to store the address for eager sends */
/* void               *MPID_Eager_address; */

/* MPID_shmem is not volatile but its contents are */
MPID_SHMEM_globmem *MPID_shmem = 0;
/* LOCAL copy of some of MPID_shmem */
MPID_SHMEM_lglobmem MPID_lshmem;
int                 MPID_myid = -1;
int                 MPID_numids = 0; /* Number local processes +1 */
/* Forward declarations */
#define MPID_SHMEM_lbarrier() MPID_SHMEM_genBarrier(&(MPID_shmem->barrier))
void MPID_SHMEM_genBarrier ANSI_ARGS( (MPID_SHMEM_Barrier_t*) );
/*
   Get an integer from the environment; otherwise, return defval.
 
int MPID_GetIntParameter( name, defval )
char *name;
int  defval;
{
    extern char *getenv();
    char *p = getenv( name );

    if (p) 
	return atoi(p);
    return defval;
} */

#pragma optimize("",off)

void MPID_SHMEM_init( argc, argv )
int  *argc;
char **argv;
{
    int numprocs, i;
    int  j;
    int memsize;
	DBG("entering MPID_SHMEM_init");

/* Make one process the default */
    numprocs = MPID_GetIntParameter( "MPICH_NP" , 1 );
	i=1;
	while(i<*argc) {
    /*for (i=1; i<*argc; i++) {*/
		if (strcmp( argv[i], "-np" ) == 0) {
			/* Need to remove both args and check for missing value for -np */
			if (i + 1 == *argc) {
				fprintf( stderr, 
					"Missing argument to -np for number of processes\n" );
				exit( 1 );
			}
			numprocs = atoi( argv[i+1] );
			argv[i] = 0;
			argv[i+1] = 0;
			MPID_ArgSqueeze( argc, argv );
			break;
		}
		i++;
    }
	
    if (numprocs <= 0 || numprocs > MPID_MAX_PROCS) {
		fprintf( stderr, "Invalid number of processes (%d) invalid\n", numprocs );
		exit( 1 );
    }

/* The environment variable MPI_GLOBMEMSIZE may be used to select memsize */
    memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );

    if (memsize < sizeof(MPID_SHMEM_globmem) + numprocs * 128)
	memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 128;

    p2p_init( numprocs, memsize );
	p2p_create_procs( numprocs - 1, *argc, argv );
	MPID_ArgSqueeze( argc, argv );

    if (!MPID_shmem) {
		fprintf( stderr, "Could not allocate shared memory (%d bytes)!\n",
			sizeof( MPID_SHMEM_globmem ) );
		exit(1);
    }


    MPID_numids = numprocs;
    MPID_MyWorldSize = numprocs;
    MPID_lshmem.mypool = MPID_shmem->pool[MPID_myid];
    for (i=0; i<MPID_numids; i++) 
      MPID_lshmem.pool[i]   = MPID_shmem->pool[i];

#ifdef RNDV_STATIC
	for(i=0;i<MPID_numids;i++) {
		for(j=0;j<MPID_numids;j++) {
			MPID_lshmem.ActBuffers[i][j] = &MPID_shmem->EagerBufs[i][j].ptr1;
		}
	}
#endif
    MPID_MyWorldRank = MPID_myid;
	p2p_wtime_init();
	MPID_Statistics_init();
	MPID_Set_statistics(0);
	DBG("leaving MPID_SHMEM_init");
}


void MPID_SHMEM_genBarrier(barrier) 
MPID_SHMEM_Barrier_t *barrier;
{
    volatile int *cnt, *cntother;

/* Figure out which counter to decrement */
    if (barrier->phase == 1) {
	cnt	     = &(barrier->cnt1);
	cntother = &(barrier->cnt2);
    }
    else {
	cnt	     = &(barrier->cnt2);
	cntother = &(barrier->cnt1);
    }


	InterlockedDecrement((LONG*)cnt);
/* Wait for everyone to to decrement it */
    while ( *cnt ) p2p_yield();

/* If process 0, change phase. Reset the OTHER counter*/
    if (MPID_myid == 0) {
	/* Note that this requires that these operations occur
	   in EXACTLY THIS ORDER */
	barrier->phase = ! barrier->phase;
	p2p_write_sync();
	*cntother = barrier->size;
    }
    else 
	while (! *cntother) p2p_yield();
}

#pragma optimize("",on)

extern BOOL finishing;

#ifdef SINGLECOPY
void CleanupHandles(void);
#endif

void MPID_SHMEM_finalize()
{

	/*fprintf(stderr,"Shutting down lfshmem device\n");*/
    fflush(stdout);
    fflush(stderr);

/* There is a potential race condition here if we want to catch
   exiting children.  We should probably have each child indicate a successful
   termination rather than this simple count.  To reduce this race condition,
   we'd like to perform an MPI barrier before clearing the signal handler.

   However, in the current code, MPID_xxx_End is called after most of the
   MPI system is deactivated.  Thus, we use a simple count-down barrier.
   Eventually, we the fast barrier routines.
 */
	finishing = TRUE;

/* Wait for everyone to finish 
   We can NOT simply use MPID_shmem->globid here because there is always the 
   possibility that some process is already exiting before another process
   has completed starting (and we've actually seen this behavior).
   Instead, we perform an additional barrier (but not an MPI barrier, because
   it is too late).
*/ 
	
    MPID_SHMEM_lbarrier();
#ifdef SINGLECOPY
	CleanupHandles();
#endif
	MPID_Runtime_statistics();
    p2p_cleanup();
} 

#ifdef WSOCK2
extern unsigned long *Global2Local, *Local2Global;
#endif

/* 
  Read an incoming control message.
 */
/*#define BACKOFF_LMT 1048576 */
#define BACKOFF_LMT 1024
int MPID_SHMEM_ReadControl( pkt, size, from, is_blocking )
MPID_PKT_TSH **pkt; /* type -> typeSH */
int        size, *from;
MPID_BLOCKING_TYPE is_blocking;
{
    register int          backoff, cnt, i, n;
    volatile int *ready;
    register MPID_PKT_TSH   *tpkt;/* type -> typeSH */
#if defined(MPID_STATISTICS) && defined(LATENCY)
	longlong_t tm;
#endif
	
	MPID_STAT_ENTRY(mpid_read_control);

    backoff = 1;
    n       = MPID_numids;
    //tpkt    = (MPID_PKT_TSH *) MPID_lshmem.mypool;
	cnt =0;
    while (1) {
		tpkt    = (MPID_PKT_TSH *) MPID_lshmem.mypool;
		for (i=0; i<n; ++i) {
		/*	fprintf( stderr, "[%d] testing [%d] = %d\n", MPID_MyWorldRank,
			i, tpkt[i].head.ready ); */
			ready = &tpkt->head.ready;
			if (MPID_PKT_READY_IS_SET(ready)){
#if defined(MPID_STATISTICS) && defined(LATENCY)
				SMI_Get_ticks(&tm);
#endif
#ifdef WSOCK2
				*from = Local2Global[i];
#else
				*from = i;
#endif
				*pkt  = tpkt ;//+ i;
				
#ifdef LATENCY
				MPID_STAT_LATENCY((*pkt)->head.time,tm);
#endif
				
				MPID_TRACE_CODE_PKT("Readpkt",i,(*pkt)->head.mode);
				/*	  fprintf( stderr, "[%d] read packet from %d\n", MPID_MyWorldRank, i ); */
				MPID_STAT_EXIT(mpid_read_control);	
				return MPI_SUCCESS;
			}
			++tpkt;
		}
		/* If nonblocking and nothing found, return 1 */
		if (!is_blocking) {
			MPID_STAT_EXIT(mpid_read_control);
			return 1;
		}
#if !defined(_WIN32) || !defined(BLOCK) || !defined(WSOCK2)
		cnt	    = backoff;
		while (cnt--) ;
		backoff = 2 * backoff;
		if (backoff > BACKOFF_LMT) backoff =  BACKOFF_LMT;
#else
		if(mixed&&(++cnt>backoff)) {
			LOG_BLOCK(0);
			//DEBUG_PRINT_MSG("Starting to wait for Signal...\n");
			if(WaitForSingleObject(MPID_Events[MPID_MyWorldRank],1)==WAIT_FAILED)
				printf("WaitFailed %d\n",GetLastError());
			LOG_BLOCK(1);
			cnt = 0;
		} 
#endif

    }
	MPID_STAT_EXIT(mpid_read_control);
    return MPI_SUCCESS;
}

void __fastcall MPID_SHMEM_GetSendPacket(MPID_PKT_TSH **pkt,int dest) {
  volatile int *destready;
  int backoff, cnt=0;

	MPID_STAT_ENTRY(mpid_getsendpkt);

#ifdef WSOCK2
  dest = Global2Local[dest];
#endif
  *pkt = (MPID_PKT_TSH*)&MPID_lshmem.pool[dest][MPID_myid];
  destready = &(*pkt)->head.ready;
  backoff = 1;
  if (MPID_PKT_READY_IS_SET(destready)) {
    while (MPID_PKT_READY_IS_SET(destready)) {
      
#ifndef WSOCK2
	  cnt = backoff;
      while (cnt--);
      backoff = 2 * backoff;
      if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
      MPID_DeviceCheck( MPID_NOTBLOCKING );
#else
	 IDLE(cnt)
#endif

    }
  }
  MPID_STAT_EXIT(mpid_getsendpkt);
}

void __fastcall MPID_SHMEM_SetPacketReady(MPID_PKT_TSH *pkt,int dest) { /* type -> typeSH */
	
	MPID_PKT_READY_SET(&pkt->head.ready);
	SIGNAL(dest);
}

int __fastcall MPID_SHMEM_SendControl( pkt, size, dest )
MPID_PKT_TSH *pkt;/* type -> typeSH */
int        size, dest;
{
  volatile int *destready;
  int backoff, cnt=0;

  MPID_STAT_ENTRY(mpid_sendcontrol);

#ifdef WSOCK2
  dest = Global2Local[dest];
#endif

  destready = &MPID_lshmem.pool[dest][MPID_myid].head.ready;
  /* Place the actual length into the packet */
  /* pkt->head.size = size; */ /* ??? */ 
  MPID_TRACE_CODE_PKT("Sendpkt",dest,pkt->head.mode);

  /*  fprintf( stderr, "[%d] dest ready flag is %d\n", 
	   MPID_MyWorldRank, *destready );*/
  backoff = 1;
  if (MPID_PKT_READY_IS_SET(destready)) {
    while (MPID_PKT_READY_IS_SET(destready)) {
      
#ifndef WSOCK2
	  cnt = backoff;
      while (cnt--);
      backoff = 2 * backoff;
      if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
      MPID_DeviceCheck( MPID_NOTBLOCKING );
#else
	 IDLE(cnt)
#endif

    }
  }
  /* Force ready == 0 until we actually do the set; this does NOT need
     to be memory synchronous */
  pkt->head.ready = 0;
  MPID_PKT_COPYIN( (void *)&MPID_lshmem.pool[dest][MPID_myid], pkt, size );
#if defined(MPID_STATISTICS) && defined(LATENCY)
	SMI_Get_ticks(&MPID_lshmem.pool[dest][MPID_myid].head.time);
#endif

  MPID_PKT_READY_SET(destready);
#ifdef WSOCK2  
  SIGNAL(Local2Global[dest]);
#endif	
  MPID_STAT_EXIT(mpid_sendcontrol);

  return MPI_SUCCESS;
}

/* 
   Return the address the destination (dest) should use for getting the 
   data at in_addr.  len is INOUT; it starts as the length of the data
   but is returned as the length available, incase all of the data can 
   not be transfered 
 */
void * MPID_SetupGetAddress( in_addr, len, dest )
void *in_addr;
int  *len, dest;
{
    void *new;
    int  tlen = *len;

    MPID_TRACE_CODE("Allocating shared space",len);

/* To test, just comment out the first line and set new to null */
 /*if(tlen < BLOCKSIZE*2) */
 new = p2p_shmalloc( tlen );
 /*else 
    new = 0; */ 
    if (!new) {
	tlen /= 2; 
	while(tlen > 0 && !(new = p2p_shmalloc(tlen))) 
		tlen /= 2;
	if (tlen == 0) {
	    p2p_error( "Could not get any shared memory for long message!",0 );
	    exit(1);
	}
 
	/* fprintf( stderr, "Message too long; sending partial data\n" ); */
	*len = tlen;
    }

    MPID_TRACE_CODE("Allocated space at",(long)new );
    return new;
}

void MPID_FreeGetAddress( addr )
void *addr;
{
    MPID_TRACE_CODE("Freeing space at",(long)addr );
    p2p_shfree( addr );
}

/*
 * Debugging support
 */
void MPID_SHMEM_Print_internals( fp )
FILE *fp;
{}

#ifdef RNDV_STATIC
int MPID_SHMEM_InitEagerBufs(numprocs)
int numprocs;
{
	int i,j;
	for(i=0;i<numprocs;i++) {
		for(j=0;j<numprocs;j++) {
			MPID_shmem->EagerBufs[i][j].ptr1=p2p_shmalloc(2*MPID_LONG_LEN);
			if(MPID_shmem->EagerBufs[i][j].ptr1==NULL) return 0;
			MPID_shmem->EagerBufs[i][j].ptr2=MPID_shmem->EagerBufs[i][j].ptr1+MPID_LONG_LEN;
		}
	}
	return 1;
}

void MPID_SHMEM_FreeRndvBuf(from,addr)
int from;
void *addr;
{
	volatile char **ptr;
	
#ifdef WSOCK2
	from = Global2Local[from];
#endif
	ptr = MPID_lshmem.ActBuffers[from][MPID_myid];
	if(!*ptr) *ptr = addr; 
	else {
	    if(ptr == &(MPID_shmem->EagerBufs[from][MPID_myid].ptr1))
		//MPID_lshmem.ActBuffers[from][MPID_myid]=&(MPID_shmem->EagerBufs[from][MPID_myid].ptr2);
		MPID_shmem->EagerBufs[from][MPID_myid].ptr2 = addr;
	    else
		//MPID_lshmem.ActBuffers[from][MPID_myid]=&(MPID_shmem->EagerBufs[from][MPID_myid].ptr1);
		MPID_shmem->EagerBufs[from][MPID_myid].ptr1 = addr;
	}
	SIGNAL(from);
}

extern void *actual_mapping_address;
extern unsigned int mem_size;

int MPID_SHMEM_IsMemoryShared(void *addr) {
    //printf("checking locality of %p (%p-%p)\n",addr,actual_mapping_address,((char*)actual_mapping_address)+mem_size);
    return ((addr>=actual_mapping_address) && 
	   ((size_t)addr<((size_t)actual_mapping_address)+mem_size));
}

void *MPID_SHMEM_GetRndvBuf(dest)
int dest;
{
	volatile char **ptr,*res;

#ifdef WSOCK2
	dest = Global2Local[dest];
#endif
	ptr=MPID_lshmem.ActBuffers[MPID_myid][dest];
	if(*ptr == NULL) return NULL;
	/*while(*ptr == NULL) idle*/;
	if(ptr == &(MPID_shmem->EagerBufs[MPID_myid][dest].ptr1))
		MPID_lshmem.ActBuffers[MPID_myid][dest]=&(MPID_shmem->EagerBufs[MPID_myid][dest].ptr2);
	else
		MPID_lshmem.ActBuffers[MPID_myid][dest]=&(MPID_shmem->EagerBufs[MPID_myid][dest].ptr1);
	res =*ptr;
	*ptr = NULL;
	return (void*)res;
}

#endif /*RNDV_STATIC*/

#ifdef SINGLECOPY
#ifndef _WIN32
#define HANDLE int
#define DWORD unsigned long
int ReadProcessMemory ANSI_ARGS(int,void *,void *,unsigned long,unsigned long* ));
int WriteProcessMemory ANSI_ARGS(int,void *,void *,unsigned long ,unsigned long*));
#endif

HANDLE GetProcessHandle(DWORD pid);

int MPID_SHMEM_CopyFromProcess(unsigned long PID,void *dest,void *rsrc,unsigned len,void *complete) {
	HANDLE hProcess;
	hProcess=GetProcessHandle(PID);
	if(!hProcess) return 0;
	ReadProcessMemory(hProcess,rsrc,dest,len,0);
	*(VOLATILE int*)complete =1;
	return 1;
}

#ifndef WSOCK2
int MAGPIE_cluster_of_process(MPI_Comm comm, int rank, int *cluster){
    *cluster = 0;
    return MPI_SUCCESS;
}

void MAGPIE_reset_cluster_info(void){
  /* do nothing -- will always be the same */
}
#endif

#endif


