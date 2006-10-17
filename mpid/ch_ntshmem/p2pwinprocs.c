/*
 * $Id$
 *
 */

#ifndef _WIN32
#error p2pwinprocs.c is for WIN32 architectures only.
#endif

#ifndef DBG
 #ifdef _DEBUG
 #include <stdio.h>
 #define DBG(m) fprintf(stderr,"%s\n",m);fflush(stderr);
 #else
 #define DBG(m)
 #endif
#endif


typedef struct {
	volatile LONG ID_Counter;
	MPID_SHMEM_Barrier_t barrier;
	volatile BOOL finished;
	HANDLE mutex;
} MPID_NT_Startup_t;

#ifdef WSOCK2
HANDLE *MPID_Events;
#endif

/* MPID_child_pid: array with process ids of local clients (shmem) */
HANDLE MPID_child_pid[MPID_MAX_PROCS];
HANDLE tmp_mutex;
static CRITICAL_SECTION CS;
BOOL finishing=FALSE;

DWORD Watchdog(void *);



void NegotiateMappingAddress ANSI_ARGS( (MPID_NT_Startup_t *,int) );
void MPID_SHMEM_genBarrier ANSI_ARGS( (MPID_SHMEM_Barrier_t*) );
void StoreProcHandle(DWORD pid,HANDLE hProcess);


#define QWORD_ALIGN(x) { \
	if ((x) & 7) \
	  x = ((((x) >> 3)+1) << 3); \
} 

#if defined(_M_IX86) && (_M_IX86 > 400)
/* On x86 Architectures (Pentium or greater) 
   call the CPUID instruction to 
   serialize instruction execution 
   (to flush the processor's write buffers)
*/
void p2p_write_sync() {
	__asm _emit 0x0F ;
	__asm _emit 0xA2 ;
}
#else 
/* We are creating code for a non-Pentium-class CPU
   80x68 (x<5) CPUs don't know CPUID and
   I don't know anything about Alpha,PPC or MIPS 
*/
void p2p_write_sync() {
	LONG dummy;
	InterlockedIncrement(&dummy);
}
#endif

#pragma optimize("",off)
void p2p_create_procs(numprocs, argc, argv)
int numprocs;
int argc;
char **argv;
{
    int i,j;
	char FatherId[24];
#ifndef WSOCK2
	BOOL rc;
	char *Environment;
	PROCESS_INFORMATION	 pInfo;
	STARTUPINFO siStartInfo;	
#endif
	MPID_NT_Startup_t *Startup;
	HANDLE hThread;
	DWORD id,size;
	DWORD flags=0;
#ifdef SHM_DEBUG
	DBG("entering p2p_create_procs");
#endif

	for(i=1;i<argc;i++) {
		if(!strcmp("-consoles",argv[i])) {
			flags=CREATE_NEW_CONSOLE;
			argv[i]=NULL;
			break;
		}
	}
	if(numprocs<1) {
		MPID_shmem=actual_mapping_address;
		p2p_lock_init(&tmp_mutex);	
		xx_init_shmalloc(((char*)actual_mapping_address)+sizeof(MPID_SHMEM_globmem),mem_size-sizeof(MPID_SHMEM_globmem));
		MPID_shmem->barrier.phase = 1;
		MPID_shmem->barrier.cnt1  = 1;
		MPID_shmem->barrier.cnt2  = 0;
		MPID_shmem->barrier.size  = 1;
		MPID_shmem->globid = 0;
		MPID_shmem->pool[0][0].head.ready = 0;
		MPID_myid = 0;
		return;
	}
	Startup=(MPID_NT_Startup_t*)actual_mapping_address;
	if(!GetEnvironmentVariable("MPICH_SHMEM_FATHER",FatherId,12)) {
#ifdef SHM_DEBUG
	    DBG("I am the local master");
#endif
		MPID_myid = 0;
		/* I am process 0, so create all child processes */
		InitializeCriticalSection(&CS);
		MPID_child_pid[0]=GetCurrentProcess();
		
		Startup->finished=TRUE;
		Startup->barrier.phase = 1;
		Startup->barrier.cnt1  = numprocs+1;
		Startup->barrier.cnt2  = 0;
		Startup->barrier.size  = numprocs+1;
		Startup->ID_Counter=0;

#ifndef WSOCK2
		p2p_lock_init(&tmp_mutex);
		Startup->mutex=tmp_mutex;
		memset(&siStartInfo,0,sizeof(siStartInfo));
		siStartInfo.cb = sizeof(STARTUPINFO); 
#ifdef _WIN64
		sprintf(FatherId,"%I64d",MemHandle);
#else
		sprintf(FatherId,"%d",MemHandle);
#endif
		SetEnvironmentVariable("MPICH_SHMEM_HANDLE",FatherId);
		sprintf(FatherId,"%d",GetCurrentProcessId());
		SetEnvironmentVariable("MPICH_SHMEM_FATHER",FatherId);

		Environment=GetEnvironmentStrings();
		WaitForSingleObject(tmp_mutex,INFINITE);
		for(i=0;i<numprocs;i++) {
			rc=CreateProcess(0,GetCommandLine(),NULL,NULL,TRUE,
				flags,Environment,NULL,&siStartInfo,&pInfo);
			if(!rc) p2p_syserror("CreateProcess failed",GetLastError());
			StoreProcHandle(pInfo.dwProcessId,pInfo.hProcess);
			CloseHandle(pInfo.hThread);
			MPID_child_pid[i+1] = pInfo.hProcess;
		}
		FreeEnvironmentStrings(Environment);
#endif

		hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)Watchdog,(void*)numprocs,0,&id);
		if(!hThread) p2p_syserror("Creation of watchdog failed",GetLastError());
		CloseHandle(hThread);
		
	} else {
		/* I am a child, so get the environment variables */
#ifdef SHM_DEBUG
	    DBG("I am a child");
#endif
#ifndef WSOCK2
		MPID_myid = InterlockedIncrement((LPLONG)&(Startup->ID_Counter));
#endif
		MPID_child_pid[1]=OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,atoi(FatherId));
		if(!MPID_child_pid[1]) {
			p2p_syserror("Cannot obtain my father's Handle",GetLastError());
		}
		StoreProcHandle(atoi(FatherId),MPID_child_pid[1]);
		hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)Watchdog,(void*)1,0,&id);
		if(!hThread) p2p_syserror("Creation of watchdog failed",GetLastError());
		CloseHandle(hThread);
	}

#ifdef WSOCK2
	while(Startup->ID_Counter) ;
#endif
	NegotiateMappingAddress(Startup,numprocs);
	MPID_shmem=actual_mapping_address;

	if(!MPID_myid) {
		
		MPID_shmem->barrier.phase = 1;
		MPID_shmem->barrier.cnt1  = numprocs+1;
		MPID_shmem->barrier.cnt2  = 0;
		MPID_shmem->barrier.size  = numprocs+1;
		MPID_shmem->globid = 0;
		
		/* The following is rough if numprocs doesn't divide the MAX_PKTS */
		/*
		* Determine the packet flush count at runtime.
		* (delay the harsh reality of resource management :-) )
		*/
		
		for (i=0; i<numprocs; i++) {
			for (j=0; j<numprocs; j++) {
				/* setup the local copy of the addresses of objects in MPID_shmem */
				MPID_shmem->pool[i][j].head.ready = 0;
			}
		}
		MPID_shmem->globid = numprocs;
	} else {
		p2p_lock( &tmp_mutex );
		
	}
	size = sizeof(MPID_SHMEM_globmem);
	QWORD_ALIGN(size);
	xx_init_shmalloc(((char*)actual_mapping_address)+size,mem_size-size);
	p2p_unlock( &tmp_mutex );

#ifdef RNDV_STATIC
	if(!MPID_myid && !MPID_SHMEM_InitEagerBufs(numprocs+1)) {
		p2p_error("Cannot allocate RndvBuffers",-1);
	}
	MPID_SHMEM_genBarrier(&(MPID_shmem->barrier));
#endif
#ifdef SHM_DEBUG
	DBG("p2p_create_procs procs finished");
#endif
}


void NegotiateMappingAddress(Startup,numprocs) 
MPID_NT_Startup_t *Startup;
int numprocs;
{
	void **Array;
	void *maximum_address,*initial_address,*res;
	int i,changed;
	SYSTEM_INFO SInfo;
	
#ifdef SHM_DEBUG
	DBG("Trying to find common address for shared-memory");
#endif
	Array=(void**)(Startup+1);
	maximum_address=initial_address=actual_mapping_address;

	GetSystemInfo(&SInfo);
#ifndef WSOCK2
	tmp_mutex=Startup->mutex;
#endif
	changed =0;
	do {
	    
	    Array[MPID_myid]=maximum_address=actual_mapping_address;
		MPID_SHMEM_genBarrier(&(Startup->barrier));
		if(!MPID_myid) {
			Startup->finished=TRUE;
			for(i=1;i<=numprocs;i++) {
				Startup->finished=(Array[i] == actual_mapping_address) && Startup->finished;
				if(maximum_address<Array[i]) maximum_address=Array[i];
			}
			
			Array[0]=maximum_address;
		}
		
		MPID_SHMEM_genBarrier(&(Startup->barrier));
		if(Array[0] != actual_mapping_address) {
			changed = 1;
			if(actual_mapping_address != initial_address)
				UnmapViewOfFile(actual_mapping_address);
			actual_mapping_address = Array[0];
			do {
				res = MapViewOfFileEx(MemHandle,FILE_MAP_ALL_ACCESS,0,0,0,actual_mapping_address);
				if(!res) {
					actual_mapping_address = ((char*)actual_mapping_address)+mem_size;
				} else {
					actual_mapping_address = res;
					break;
				}

			} while(actual_mapping_address < SInfo.lpMaximumApplicationAddress);
		}
		MPID_SHMEM_genBarrier(&(Startup->barrier));
		if(actual_mapping_address >= SInfo.lpMaximumApplicationAddress)
				p2p_error("Cannot find common mapping address",-1);
	} while (!(Startup->finished));
	MPID_SHMEM_genBarrier(&(Startup->barrier));
	if(changed) UnmapViewOfFile(initial_address);
}

#pragma optimize("",on)

void p2p_cleanup()
{
	int i;
	finishing=TRUE;
	if (p2p_shmem_lock != NULL)
	    CloseHandle(*p2p_shmem_lock);
	UnmapViewOfFile(freep);
	CloseHandle(MemHandle);
	if(MPID_myid) return;
	for(i=1;i<MPID_numids;i++) {
		CloseHandle(MPID_child_pid[i]);
	}
	if(!MPID_myid) 
		DeleteCriticalSection(&CS);
}


void p2p_kill_procs()
{
	
}

#ifdef _WIN64
__int64 p2p_proc_info( id, host_name, image_name )
#else
int p2p_proc_info( id, host_name, image_name )
#endif
int id;
char **host_name;
char **image_name;
{
#ifdef _WIN64
	return (__int64) MPID_child_pid[id-1];
#else
	return (int) MPID_child_pid[id-1];
#endif
}


DWORD Watchdog(void *np) {
	DWORD res;
	unsigned long size=(unsigned long)np;
	res=WaitForMultipleObjects(size,MPID_child_pid+1,FALSE,INFINITE);
	if(res==WAIT_FAILED) {
		p2p_syserror("Wait for process failed",GetLastError());
		return 2;
	}
	if(finishing) return 0;
	fprintf(stderr,"Detected crash of a process terminating...\n");
	p2p_kill_procs();
	ExitProcess(-1);
	// To avoid compiler warning
	return 0;
}

