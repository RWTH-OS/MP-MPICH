#if !defined(linux)
#include <synch.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#if !defined(linux)
#include <sys/systeminfo.h>
#include <stropts.h>
#include <poll.h>
#include <sys/mman.h>

#else

#include <sys/ioctl.h>
#include <fstream>
#endif

#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <map>

#ifndef __cplusplus
#error "filemap.cc: this file is C++."
#endif
//---------------------------------------------------------------------------------------
#define _DEBUG_EXTERN_REC
#include "mydebug.h"
#include "nt2unix.h"
#include "unixexception.h"
#include "filemap.h"
#include "debugnt2u.h"

using namespace std;

vector<FileMapping> FileMappings;

#ifdef __cplusplus
extern "C" {
#endif
  //static vector<FileMapping> FileMappings;

WINBASEAPI
LPVOID
WINAPI
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    )
{
    return MapViewOfFileEx(hFileMappingObject,
			   dwDesiredAccess,
			   dwFileOffsetHigh,
			   dwFileOffsetLow,
			   dwNumberOfBytesToMap,
			   0);
}

WINBASEAPI
LPVOID
WINAPI
MapViewOfFileEx(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap,
    LPVOID lpBaseAddress
    )
{
    DSECTION("MapViewOfFileEx");
    int prot = 0, flags = 0; 
    LPVOID ret;
    HandleInfo* Map; 
    
    DSECTENTRYPOINT;

    // *******
    // Map = (HandleInfo*) malloc(sizeof(HandleInfo));
    Map = (HandleInfo*)hFileMappingObject;  // *******
    
    DBG("Entering MapViewOfFileEx(). DesiredAccess== "<<dwDesiredAccess<<" FILE_MAP_COPY=="<<FILE_MAP_COPY);  
    if (dwFileOffsetHigh > 0)
	DBG("MapViewOfFileEx(): ignoring dwFileOffsetHigh");
    
    // Filter the protection bits ...
    prot = dwDesiredAccess & FILE_MAP_ALL_ACCESS; 
    // ... and mapping flags: 
    flags = ((dwDesiredAccess & FILE_MAP_COPY)==FILE_MAP_COPY) ? MAP_PRIVATE : MAP_SHARED;
    DBG("mapping "<<(flags==MAP_PRIVATE?"private":"shared"));
    /* MAP_SHARED does not work correctly under Solaris SPARC 2.5.1 */
    if (lpBaseAddress)
	flags |= MAP_FIXED; 
    
    // Search and update the mapping in the vector.
    vector<FileMapping>::iterator i=FileMappings.begin();
    while(i != FileMappings.end() && i->hFileMappingObject != Map->obj) // *******
	i++;
    if (i != FileMappings.end()) {
	if (dwNumberOfBytesToMap) {
	    DBG("Setting dwNumberOfBytesToMap to "<<dwNumberOfBytesToMap);
	    i->dwNumberOfBytesToMap = dwNumberOfBytesToMap;
	}
    } else {
	DBG("MapViewOfFileEx(): mapping not found.");
	DSECTLEAVE
	    return 0; 
    } 
#if defined(_DEBUG) && !defined(SYSVSHM) 
    fprintf(stderr, "mmap(): lpBaseAddress == %x, prot == %d, flags == %d\n",
	    lpBaseAddress, prot, flags);
    fprintf(stderr, "  dwNumberOfBytesToMap == %d, dwFileOffsetLow == %d\n",
	    i->dwNumberOfBytesToMap, dwFileOffsetLow); 
#endif 
    if(i->type==FILESEGMENT) {
  	if ((ret = (LPVOID)mmap((caddr_t)lpBaseAddress, 
				(size_t)i->dwNumberOfBytesToMap,    
				prot, flags, (int)Map->obj,    // *******
				(off_t)dwFileOffsetLow)) == (LPVOID)MAP_FAILED) {             
	    perror("MapViewOfFileEx(): mmap() failed. \n");
	    DSECTLEAVE
		return 0;
  	}
    } else {
	ret=shmat((int)Map->obj,(const void *)lpBaseAddress,0);    // *******
	if((int)ret==-1) {
	    perror("shmat");
	    DSECTLEAVE
		return 0;
	}
    }
    
    if(i->refcnt==0) {
	// Initialize the mapping.
	DBG("Initializing mapping ...");
	if (mprotect((caddr_t)ret, (size_t)i->dwNumberOfBytesToMap, PROT_WRITE) == -1)
	    perror("mprotect()"); 
	/*
	  memset(ret, 0x00, (size_t)i->dwNumberOfBytesToMap);
	*/
	/* The following memset() is absolutely necessary,
	   even for NFS file systems !!!! */
	memset(ret, 0x00, 4);
	DBG("... done. prot is now "<< prot);
    }
    if (mprotect((caddr_t)ret, (size_t)i->dwNumberOfBytesToMap, prot) == -1)
	perror("mprotect()"); 
    
    DBG("MapViewOfFileEx(): set lpBaseAddress to "<<ret);
    i->BaseAddresses = (LPVOID*)realloc(i->BaseAddresses,
					(i->refcnt+1)*sizeof(LPVOID));
    i->BaseAddresses[i->refcnt++] = ret;  
    
    DSECTLEAVE
	return ret;  
}
                            
void DeleteFilemapping(vector<FileMapping>::iterator &i) {
    DSECTION("DeleteFilemapping");

    DSECTENTRYPOINT;

    if(!i->refcnt) {
	if(i->type==SYSVSEGMENT) {
	    if(shmctl((int)(i->hFileMappingObject),IPC_RMID,0)<0) {
		perror("shmctl");
	    }
	} else {
	    close((int)(i->hFileMappingObject));
       }
	if(i->BaseAddresses) free(i->BaseAddresses);
	FileMappings.erase(i);
    }
   DSECTLEAVE;
}

                            
WINBASEAPI
BOOL
WINAPI
UnmapViewOfFile(
    LPCVOID lpBaseAddress
    )
{
    DSECTION("UnmapViewOfFile");
    bool found;
    int j;
    
    DSECTENTRYPOINT;
    
    // Search the mapping according to the lpBaseAddress.
    DBG("UnmapViewOfFile(): searching for "<<lpBaseAddress);
    vector<FileMapping>::iterator i=FileMappings.begin();
    found = false;
    while(i != FileMappings.end() && !found ) {
	for(j=0;j<i->refcnt;j++) {
	    if(i->BaseAddresses[j] == (LPVOID)lpBaseAddress) {
		found = true;
		break;
	    }
	}
    	if(!found) i++;
    }
    if (found) {
	// found
	if(i->type==FILESEGMENT) {
	    if (munmap((caddr_t)lpBaseAddress, (size_t)(i->dwNumberOfBytesToMap))<0) {
		DBG("UnmapViewOfFile(): munmap() failed.");
		DSECTLEAVE
		    return FALSE;
	    }
	} else {
	    shmdt((char*)lpBaseAddress);
	}
	memmove(i->BaseAddresses+j,i->BaseAddresses+j+1,
		(i->refcnt-j-1)*sizeof(LPVOID));
	i->refcnt--;
	
	if (i->Closed) DeleteFilemapping(i);
	DSECTLEAVE
	    return TRUE; 
    } else {
	DBG("UnmapViewOfFile(): mapping not found.");
    } 
    
    DSECTLEAVE
	return FALSE;
}


WINBASEAPI
HANDLE
WINAPI
CreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    )
{
    DSECTION("CreateFileMappingA");
    
    DSECTENTRYPOINT;
    
    DBG("Entering CreateFileMappingA(), hFile == "<<hFile);
    
    HandleInfo* Map;
    Map = (HandleInfo*) malloc(sizeof(HandleInfo));
    Map->handleType = HANDLETYPE_FILEMAPPING;
    Map->refcnt++;
    
    int fildes;
    struct FileMapping thisMapping;   
    
    // Initialize FileMapping structure
    thisMapping.BaseAddresses = 0;
    thisMapping.dwNumberOfBytesToMap = 0;
    thisMapping.hFileMappingObject = 0; 
    thisMapping.FileName[0] = 0; 
    thisMapping.refcnt = 0; 
    thisMapping.Closed = FALSE;   
    if (lpFileMappingAttributes)
	DBG("CreateFileMappingA(): lpFileMappingAttributes not supported.");
    
    if (hFile == (HANDLE)0xFFFFFFFF) {
	// user must specify file size
	if ((dwMaximumSizeHigh < 1) && (dwMaximumSizeLow < 1))
	    DBG("CreateFileMappingA(): dwMaximumSize == 0");
	DBG("Creating segment of size "<<dwMaximumSizeLow);
	// Open a file of size dwMaximumSizeLow.
    if (dwMaximumSizeHigh > 0)
	DBG("CreateFileMappingA(): ignoring dwMaximumSizeHigh");
#ifdef SYSVSHM    
    fildes=shmget(IPC_PRIVATE,dwMaximumSizeLow,0600);
      if(fildes==-1) {
	  perror("shmget()");
	  DBG("Cannot SHMOpen\n");
	  
	  DSECTLEAVE
	      return 0;
      }
      SolarisException.installHandler();
      thisMapping.type=SYSVSEGMENT;
#else
      thisMapping.type=FILESEGMENT;
#endif
      if (!lpName) {
#ifndef SYSVSHM
	  
	  // We dont use tmpfile(), since it obviously generates only
	  // one file per process.
	  // NOTE: Mapping with MAP_SHARED doesnot
	  // work correctly on a non-NFS file system under Solaris SPARC. 
#ifdef NT2UNIX_USE_TEMPNAM
	  lpName = tempnam(PATH_PREFIX,"svm__"); 
	  if (!lpName) {
	      DBG("CreateFileMappingA: tempnam() failed.");
	      DSECTLEAVE
		  return 0; 
	  }      
	  DBG("CreateFileMappingA(): creating temporary file: "<<lpName<<" Prefix is: "<<PATH_PREFIX);
	  fildes = open((const char*)lpName,
			O_CREAT | O_RDWR, 0600); 
	  if (!fildes) {
	      DBG("CreateFileMappingA(): cannot create file.");
	      DSECTLEAVE
		  return 0;
	  }
#else
	  { 
	      char* szTemp = (char*) malloc(sizeof(char) * PATH_MAX);
	      
	      if (!szTemp) {
		  DBG("CreateFileMappingA: malloc() failed.");
		  printf("CreateFileMappingA: malloc() failed.\n");
		  DSECTLEAVE
		      return 0;
	      }      
	      /*     sprintf(szTemp, "%s/%s",PATH_PREFIX,"svm__XXXXXX"); */
	      sprintf(szTemp, "%s","svm__XXXXXX");
	      fildes = mkstemp(szTemp);
	      if (fildes == -1) {
		  DBG("CreateFileMappingA(): cannot create file.");
		  printf("CreateFileMappingA(): cannot create file\n");
		  free(szTemp);
		  DSECTLEAVE
		      return 0;
	      }
	      lpName = szTemp;
	  }
#endif
	  
	  // make sure the file is really of the right size. 
	  if(ftruncate(fildes, (off_t)((dwMaximumSizeLow/8192+1)*8192) ) == -1)
	      perror("CreateFileMappingA(): ftruncate()");
	  unlink((const char *)lpName);
#endif
      } else {
#ifndef SYSVSHM
	  fildes = open((const char*)lpName, O_RDWR);
#endif       
	  strcpy(thisMapping.FileName, lpName); 
      }
      
      if (!fildes) {
	  DBG("CreateFileMappingA(): cannot create / open file.");
	  DSECTLEAVE
	      return 0;
      }
      
      thisMapping.hFileMappingObject = (HANDLE)fildes; 
      thisMapping.dwNumberOfBytesToMap = dwMaximumSizeLow;   
      FileMappings.push_back(thisMapping); 
      Map->obj = (HANDLE)fildes; // **************
      
      DSECTLEAVE
	  return (HANDLE)Map; // *** hier stand return (HANDLE)fildes
    }
    fildes = (int)hFile; 
    thisMapping.hFileMappingObject = hFile;
    thisMapping.type=FILESEGMENT;
    if(!dwMaximumSizeLow) {
	// In this case, we must take the size of the specified file
	// as the size of the mapping. 
	if ( (off_t)(dwMaximumSizeLow = lseek(fildes, 0, SEEK_END)) == -1) 
	    perror("CreateFileMappingA(): lseek()"); 
	if (dwMaximumSizeLow < 1) {
	    DBG("CreateFileMappingA(): cannot create empty file mapping.");
	    DSECTLEAVE
		return 0;
	}
    }
    
    thisMapping.dwNumberOfBytesToMap = dwMaximumSizeLow; 
    FileMappings.push_back(thisMapping);
    
    Map->obj = hFile; // ****************
 
    DSECTLEAVE
	return (HANDLE)Map; // *** hier stand return hFile
}
#ifdef __cplusplus 
}
#endif
