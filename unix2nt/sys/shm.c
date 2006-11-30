/* $Id$ */

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include "env/general_definitions.h"
#include "env/smidebug.h"


typedef struct _HandleChain *PtrHandleChain;
struct _HandleChain {
	PtrHandleChain next;
	HANDLE handle;
	key_t key;
	void *address;
	unsigned long size;
} HandleChain;

static PtrHandleChain start = NULL;


/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
int shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
	int iReturn = 0;
	SYSTEM_INFO SystemInfo;
	PtrHandleChain dummy = start;

	switch(cmd)
	{
		case IPC_RMID: 
			while ((dummy != NULL) && (dummy->key != shmid))
				dummy = dummy->next;
			if (dummy != NULL)
				CloseHandle(dummy->handle);
			else iReturn = -1;
			break;
		case IPC_STAT:
			while ((dummy != NULL) && (dummy->key != shmid))
				dummy = dummy->next;
			if (dummy != NULL)
			    buf->shm_segsz = dummy->size;
			else iReturn = -1;
			break;
	}
	return(iReturn);
}


/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
int shmget(key_t key, int size, int shmflg)
{
    PtrHandleChain dummy;
    HANDLE hFile = (HANDLE)0xFFFFFFFF;
    DWORD flProtect;
    char name[100];
    
    if ((key == IPC_PRIVATE) || ((IPC_CREAT & shmflg) == IPC_CREAT))
    {
	if ((shmflg & O_RDONLY) == O_RDONLY)
	    flProtect = PAGE_READONLY;
	else 
	    flProtect = PAGE_READWRITE;
	
	if (key == IPC_PRIVATE)
	{
	    do {
		key++;
		_itoa(key, name, 10);
		if ((hFile != (HANDLE)0xFFFFFFFF) && (hFile != NULL))
		    CloseHandle(hFile);
		hFile = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, flProtect, 0, size, name);
	    } while (GetLastError() != 0);
	} else {
	    _itoa(key, name, 10);
	    hFile = CreateFileMapping(hFile, NULL, flProtect, 0, size, name);
	}
	
	if (hFile == NULL)
	{
	    errno = GetLastError();
	    return(-1);
	}
    } else {
	if ((shmflg & O_RDONLY) == O_RDONLY)
	    flProtect = FILE_MAP_READ;
	else
	    flProtect = FILE_MAP_WRITE;
	
	_itoa(key, name, 10);
	hFile = OpenFileMapping(flProtect, TRUE, name);
	if (hFile == NULL)
	{
	    errno = GetLastError();
	    return(-1);
	}
    }
    
    if (start == NULL)
    {
	start = (PtrHandleChain) malloc(sizeof(HandleChain));
	if (start == NULL)
	    return(-1);
	dummy = start;
    } else
    {
	dummy = start;
	while(dummy->next != NULL)
	    dummy = dummy->next;
	dummy->next = (PtrHandleChain) malloc(sizeof(HandleChain));
	if (dummy == NULL)
	    return(-1);
	dummy = dummy->next;
	
    }
    dummy->next = NULL;
    dummy->handle = hFile;
    dummy->key = key;
    dummy->address = NULL;
    dummy->size = size;
    
    return((int) key);
}


/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
void *shmat(int shmid, void *shmaddr, int shmflg)
{
    PtrHandleChain dummy = start;
    DWORD dwDesiredAccess;
    HANDLE hFile = NULL;
    int res;
    
    DSECTION("shmat");
    DSECTENTRYPOINT;
    
    if ((shmflg&SHM_RDONLY) == SHM_RDONLY)
	dwDesiredAccess = FILE_MAP_READ;
    else 
	dwDesiredAccess = FILE_MAP_WRITE;
    
    while ((dummy != NULL) && (dummy->key != shmid))
	dummy = dummy->next;

    if(dummy == NULL) {
	/* We have to open the mapping first.
	Hopefully someone else alredy created it...*/
	res = shmget(shmid,0,0);
	if(res <0) {
	    DWARNING("shmget failed for key");
	    DSECTLEAVE
	    return (void *) -1;
	}
	dummy = start;
	while ((dummy != NULL) && (dummy->key != shmid))
	    dummy = dummy->next;

    }
    
    if (dummy != NULL)
    {
	if (shmaddr == NULL)
	    shmaddr = MapViewOfFile(dummy->handle, dwDesiredAccess, 0, 0, 0);
	else 
	{
	    if (MapViewOfFileEx(dummy->handle, dwDesiredAccess, 0, 0, 0, shmaddr) == NULL)
	    {
		errno = GetLastError();
		shmaddr = (void *) -1;
	    }
	}
	dummy->address = shmaddr;
    } 
    
    //_smi_ll_barrier();
    DNOTICEP("mapped to address ",shmaddr);
    DSECTLEAVE
    return(shmaddr);
}


/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
int shmdt (void *shmaddr)
{
    PtrHandleChain dummy = start;
    
    while((dummy != NULL) && (dummy->address != shmaddr))
    {	 
	dummy = dummy->next;
    }
    if (dummy != NULL)
    {
	if (UnmapViewOfFile(shmaddr) == FALSE)
	{
	    errno = GetLastError();
	    return(-1);
	}
	
    } else {
	errno = 22;
	return -1;
    }
    /*if (dummy==0) fprintf(stderr,"SCHEISSE\n");*/
    return(0);
}

