#ifndef __NT2UNIX_H__
#define __NT2UNIX_H__

/* 
 *   nt2unix 
 *
 *   This interface maps some NT Win32 system calls and structures
 *   to the appropriate UNIX (Solaris) calls and structures.
 *   You need a C++ compiler along with the STL (standard template
 *   library) to compile the source. 
 * 
 *   NOTE: definitions marked with DUMMY are not identical
 *   with the original Win32 definitions !!!
 *
 *
 *
 */

#if defined(sparc)
typedef unsigned int Unsigned;
#endif

/* Avoid collision with STL's type queue
   / #define queue _queue*/
 
#if defined(linux)
#ifndef __POSIX_THREADS__
#define __POSIX_THREADS__
#endif
#endif

#ifdef __POSIX_THREADS__
#include <pthread.h>
#else
#include <thread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/mman.h>
/*#include <arpa/inet.h>*/
#include <netdb.h>

#if !defined(linux)
#include <ucontext.h>
#endif

#include <unistd.h>

#undef queue

#ifdef __cplusplus
extern "C" {
#endif

#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define _alloca alloca
#define __stdcall
/* Basic Win32 Data Type emulation --------------------------------------- */

#define MAX_PATH 255 /* DUMMY */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define CONST const
#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif
typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH, *LPCTSTR;

typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CHAR *LPTSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;

#define WINBASEAPI
#define WINAPI
#define near
#define far
#define FAR
#define PASCAL
#define NEAR
#define __cdecl
#define _DWORD_DEFINED
typedef int (FAR WINAPI *FARPROC)(); 
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int        UINT;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void far            *LPVOID;
typedef CONST void far      *LPCVOID;
typedef void                *PVOID;
typedef double              DWORDLONG;
typedef long long				 LONGLONG;

typedef union _LARGE_INTEGER {
  /*
  struct {
    DWORD LowPart;
    LONG HighPart;
  };
  */
  struct {
#ifdef sparc
    LONG HighPart;
    DWORD LowPart;
#else
    DWORD LowPart;
    LONG HighPart;
#endif
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
  /*
  struct {
    DWORD LowPart;
    LONG HighPart;
  };
  */
#ifdef sparc
    LONG HighPart;
    DWORD LowPart;
#else
    DWORD LowPart;
    LONG HighPart;
#endif
  DWORDLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;
                                                    
typedef struct _LUID {
  DWORD LowPart;
  LONG HighPart;
} LUID, *PLUID;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef void *HANDLE;


typedef HANDLE FAR          *LPHANDLE;
typedef LPVOID LPLDT_ENTRY;			  	/* DUMMY */
typedef LPVOID LPSECURITY_ATTRIBUTES;
typedef int SECURITY_ATTRIBUTES; 			/* DUMMY */
typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
#define WINBASEAPI

#define RESTRICTED_POINTER

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY * volatile Flink;
   struct _LIST_ENTRY * volatile Blink;
} LIST_ENTRY, *PLIST_ENTRY, *RESTRICTED_POINTER PRLIST_ENTRY;

typedef struct _SINGLE_LIST_ENTRY {
    struct _SINGLE_LIST_ENTRY *Next;
} SINGLE_LIST_ENTRY, *PSINGLE_LIST_ENTRY;

typedef struct _RTL_CRITICAL_SECTION_DEBUG {
    WORD   Type;
    WORD   CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    DWORD EntryCount;
    DWORD ContentionCount;
    DWORD Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;

#define RTL_CRITSECT_TYPE 0
#define RTL_RESOURCE_TYPE 1

typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    DWORD Reserved;
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION_DEBUG CRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG PCRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG LPCRITICAL_SECTION_DEBUG;

typedef struct _SYSTEM_INFO {
    /* union { */                   /* DUMMY */
            DWORD dwOemId;      
            /* struct { */	    /* DUMMY */
               WORD wProcessorArchitecture;
               WORD wReserved;
            /* }; */                /* DUMMY */         
    /*       };*/                   /* DUMMY */
    /* 
     |   The above anonymous union/struct are commented
     |   out, because g++ and CC don't like it. 
     */

    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

WINBASEAPI
HANDLE
WINAPI
LoadLibrary(char *Library);

WINBASEAPI
VOID
WINAPI
FreeLibrary(HANDLE Library);

WINBASEAPI
VOID*
WINAPI
GetProcAddress(HANDLE Library,char *FuncName);

  /* ----------------------------------------------------------------------*/

  /* not used yet*/
typedef struct _HandleInfo {
  DWORD handleType; 	/* handletype of this handle (see below)*/
  void *obj; 		/* pointer to handled object*/
  DWORD refcnt; 
}HandleInfo;

#ifdef __POSIX_THREADS__
#define cond_t pthread_cond_t
#define mutex_t pthread_mutex_t
#endif

typedef struct _Event_Struct_Typ
  {
#ifdef __POSIX_THREADS__
    pthread_cond_t condition;
    pthread_mutex_t mutex;
#else
    cond_t condition;
    mutex_t mutex;
#endif
    int count;
    BOOL State;
    BOOL Manual;
  }Event_Struct_Typ;

  /* not used yet*/
#define HANDLETYPE_CONSOLE	0	// not supported
#define HANDLETYPE_EVENTFILE	1
#define HANDLETYPE_FILEMAPPING	2
#define HANDLETYPE_MUTEX	3
#define HANDLETYPE_NAMEDPIPE	4
#define HANDLETYPE_PROCESS	5
#define HANDLETYPE_SEMAPHORE	6
#define HANDLETYPE_THREAD	7
#define HANDLETYPE_TOKEN	8

  /*-----------------------------------------------------------------------*/


/* Threading functions --------------------------------------------------- */

#ifdef __POSIX_THREADS__
#define THR_SUSPENDED 1 /* DUMMY */

  #ifdef linux
    #define thr_yield sched_yield
  #else 
    #define thr_yield yield
  #endif
#endif

#define CREATE_SUSPENDED THR_SUSPENDED /* DUMMY */

#define _beginthreadex (unsigned long)::CreateThread
#define _endthreadex(ret) return ret

WINBASEAPI
HANDLE
WINAPI
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

WINBASEAPI
HANDLE
WINAPI
CreateRemoteThread(
    HANDLE hProcess,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

WINBASEAPI
HANDLE
WINAPI
GetCurrentThread(
    VOID
    );

WINBASEAPI
DWORD
WINAPI
GetCurrentThreadId(
    VOID
    );

WINBASEAPI
DWORD
WINAPI
SetThreadAffinityMask(
    HANDLE hThread,
    DWORD dwThreadAffinityMask
    );

WINBASEAPI
BOOL
WINAPI
SetThreadPriority(
    HANDLE hThread,
    int nPriority
    );

WINBASEAPI
int
WINAPI
GetThreadPriority(
    HANDLE hThread
    );

WINBASEAPI
BOOL
WINAPI
GetThreadTimes(
    HANDLE hThread,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    );

WINBASEAPI
VOID
WINAPI
ExitThread(
    DWORD dwExitCode
    );

VOID ExitProcess(UINT uExitCode);

WINBASEAPI
BOOL
WINAPI
TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode
    );


WINBASEAPI
BOOL
WINAPI
GetExitCodeThread(
    HANDLE hThread,
    LPDWORD lpExitCode
    );


WINBASEAPI
BOOL
WINAPI
GetThreadSelectorEntry(
    HANDLE hThread,
    DWORD dwSelector,
    LPLDT_ENTRY lpSelectorEntry
    );

#define MAXCHAR 0x7f
#define MAXIMUM_SUSPEND_COUNT MAXCHAR
DWORD ResumeThread(HANDLE hThread);
DWORD SuspendThread(HANDLE hThread);

/* Thread Local Storage (TLS) Functions ----------------------------- */

WINBASEAPI
DWORD
WINAPI
TlsAlloc(
    VOID
    );

#define TLS_OUT_OF_INDEXES (DWORD)0xFFFFFFFF

WINBASEAPI
LPVOID
WINAPI
TlsGetValue(
    DWORD dwTlsIndex
    );

WINBASEAPI
BOOL
WINAPI
TlsSetValue(
    DWORD dwTlsIndex,
    LPVOID lpTlsValue
    );

WINBASEAPI
BOOL
WINAPI
TlsFree(
    DWORD dwTlsIndex
    );

/* Handle Functions ------------------------------------------------- */
#define DUPLICATE_SAME_ACCESS 0
#define SYNCHRONIZE 0

WINBASEAPI
BOOL
WINAPI
CloseHandle(
    HANDLE hObject
    );

WINBASEAPI
BOOL
WINAPI
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    );

WINBASEAPI
BOOL
WINAPI
GetHandleInformation(
    HANDLE hObject,
    LPDWORD lpdwFlags
    );

#define INFINITE 0xFFFFFFFF

DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD dwMilliseconds
  );

DWORD WaitForMultipleObjects(DWORD nCount, 
			     CONST HANDLE *lpHandles, 
			     BOOL bWaitAll,
			     DWORD dwMilliseconds);  

/* Process Functions ------------------------------------------------------ */
#define CREATE_NEW_CONSOLE 0

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess; 
    HANDLE hThread; 
    DWORD dwProcessId; 
    DWORD dwThreadId; 
} PROCESS_INFORMATION;

typedef struct _STARTUPINFO {
    DWORD   cb; 
    LPTSTR  lpReserved; 
    LPTSTR  lpDesktop; 
    LPTSTR  lpTitle; 
    DWORD   dwX; 
    DWORD   dwY; 
    DWORD   dwXSize; 
    DWORD   dwYSize; 
    DWORD   dwXCountChars; 
    DWORD   dwYCountChars; 
    DWORD   dwFillAttribute; 
    DWORD   dwFlags; 
    WORD    wShowWindow; 
    WORD    cbReserved2; 
    LPBYTE  lpReserved2; 
    HANDLE  hStdInput; 
    HANDLE  hStdOutput; 
    HANDLE  hStdError; 
} STARTUPINFO, *LPSTARTUPINFO;

WINBASEAPI
DWORD
WINAPI
GetCurrentProcessId(VOID);

WINBASEAPI
HANDLE
WINAPI
GetCurrentProcess(VOID);

WINBASEAPI
HANDLE
WINAPI
OpenProcess(DWORD dwDesiredAccess, /* access flag */
	    BOOL bInheritHandle,   /* handle inheritance flag */
	    DWORD dwProcessId);    /* process identifier */

/* Thread-Synchronization Functions --------------------------------------- */
WINBASEAPI
VOID
WINAPI
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
HANDLE
WINAPI
CreateMutex(LPSECURITY_ATTRIBUTES lpMutex_Attributes,
	    BOOL bInitialOwner,
	    LPCTSTR lpName);

WINBASEAPI
BOOL
WINAPI
ReleaseMutex(HANDLE hMutex);

WINBASEAPI
HANDLE
WINAPI
CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, 
	    BOOL bManualReset, 
	    BOOL bInitialState,
	    LPCTSTR lpName);
WINBASEAPI
BOOL
ResetEvent (HANDLE hEvent);	   

WINBASEAPI
BOOL
WINAPI
SetEvent (HANDLE hEvent);

WINBASEAPI
HANDLE
CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount,
		LPCTSTR lpName);

WINBASEAPI
BOOL
ReleaseSemaphore(HANDLE hSemaphore, 
		 LONG lReleaseCount, 
		 LPLONG lpPreviousCount);

/* Virtual Memory Management Functions ------------------------------------ */
/* DUMMY */
#define PAGE_READONLY		PROT_READ
#define PAGE_READWRITE		(PROT_READ | PROT_WRITE)
#define PAGE_NOACCESS		PROT_NONE
#define PAGE_EXECUTE		PROT_EXEC
#define PAGE_EXECUTE_READ	(PROT_EXEC|PROT_READ)
#define PAGE_EXECUTE_READWRITE	(PROT_EXEC|PROT_READ|PROT_WRITE)
#define	PAGE_GUARD		0x100 /* not supported. */
#define PAGE_NOCACHE		0x200 /* not supported. */

/* From winnt.h */
#define MEM_COMMIT           0x1000     
#define MEM_RESERVE          0x2000     
#define MEM_DECOMMIT         0x4000     
#define MEM_RELEASE          0x8000     
#define MEM_FREE            0x10000     
#define MEM_PRIVATE         0x20000     
#define MEM_MAPPED          0x40000     
#define MEM_RESET           0x80000     
#define MEM_TOP_DOWN       0x100000     

#define SEC_FILE           0x800000     
#define SEC_IMAGE         0x1000000     
#define SEC_RESERVE       0x4000000     
#define SEC_COMMIT        0x8000000     
#define SEC_NOCACHE      0x10000000     
#define MEM_IMAGE         SEC_IMAGE     

BOOL VirtualLock(LPVOID lpAddress, DWORD dwSize);
BOOL VirtualUnlock(LPVOID lpAddress, DWORD dwSize);

WINBASEAPI
LPVOID
WINAPI
VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );

WINBASEAPI
BOOL
WINAPI
VirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );

WINBASEAPI
BOOL
WINAPI
VirtualProtect(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    );

/* File Mapping Functions ------------------------------------------- */

#define FILE_MAP_COPY		0x80
#define FILE_MAP_WRITE    	PROT_WRITE  
#define FILE_MAP_READ      	PROT_READ
#define FILE_MAP_ALL_ACCESS 	(PROT_READ|PROT_WRITE)


WINBASEAPI
LPVOID
WINAPI
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );


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
    ); 


WINBASEAPI
BOOL
WINAPI
UnmapViewOfFile(
    LPCVOID lpBaseAddress
    );

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
    );

#define CreateFileMapping  CreateFileMappingA

  /* Misc ----------------------------------------------------------- */

VOID QueryPerformanceCounter(LARGE_INTEGER *v);
VOID QueryPerformanceFrequency(LARGE_INTEGER *f);

DWORD FormatMessage (DWORD dwFlags,
		     LPVOID lpSource, 
		     DWORD dwMessageId, 
		     DWORD dwLanguageId, 
		     LPTSTR lpBuffer, 
		     DWORD nSize, 
		     va_list *Arguments);

#define MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) 0

DWORD GetLastError(VOID); 
VOID SetLastError(DWORD dwErrCode); 
VOID Sleep(DWORD dwMilliseconds);

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0
#define FORMAT_MESSAGE_FROM_SYSTEM 1
#define FORMAT_MESSAGE_FROM_STRING 3
#define LANG_NEUTRAL 4
#define SUBLANG_DEFAULT 5
#define SEM_FAILCRITICALERRORS      0x0001
#define SEM_NOGPFAULTERRORBOX       0x0002
#define SEM_NOALIGNMENTFAULTEXCEPT  0x0004
#define SEM_NOOPENFILEERRORBOX      0x8000
UINT SetErrorMode(UINT uMode); 

#define PROCESSOR_INTEL_386     386
#define PROCESSOR_INTEL_486     486
#define PROCESSOR_INTEL_PENTIUM 586
#define PROCESSOR_MIPS_R4000    4000
#define PROCESSOR_ALPHA_21064   21064

#define PROCESSOR_ARCHITECTURE_INTEL 0
#define PROCESSOR_ARCHITECTURE_MIPS  1
#define PROCESSOR_ARCHITECTURE_ALPHA 2
#define PROCESSOR_ARCHITECTURE_PPC   3
#define PROCESSOR_ARCHITECTURE_SPARC 4		/* DUMMY */
#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xFFFF

WINBASEAPI
VOID
WINAPI
GetSystemInfo(LPSYSTEM_INFO lpSystemInfo); 

#define MAX_COMPUTERNAME_LENGTH MAXHOSTNAMELEN /* DUMMY */
WINBASEAPI
BOOL
WINAPI
GetComputerName(LPTSTR lpBuffer, LPDWORD nSize);

  BOOL SetEnvironmentVariable( LPCTSTR lpName,   /* address of environment variable name */
			       LPCTSTR lpValue); /* address of new value for variable*/

  DWORD GetEnvironmentVariable(LPCTSTR lpName,   /* address of environment variable name */
			       LPTSTR lpBuffer,  /* address of buffer for variable value */
			       DWORD nSize);     /* size of buffer, in characters*/

WINBASEAPI LPSTR WINAPI GetEnvironmentStrings(
    						VOID
    					     );
WINBASEAPI LPSTR WINAPI GetCommandLine(  
    					VOID
    				      );

int stricmp (const char* string1, const char* string2);

/* IP Helper Functions -------------------------------------------- */

#include "iphlpapi.h"

/* DWORD */
/* WINAPI */
/* GetIfEntry( PMIB_IFROW   pIfRow); */

/* DWORD */
/* WINAPI */
/* GetIpAddrTable( PMIB_IPADDRTABLE pIpAddrTable, */
/*                 PULONG          pdwSize, */
/*                 BOOL             bOrder); */


/* WinSock API ---------------------------------------------------- */

/* Caveat: Solaris socket functions are Safe, but not MT-Safe. 
   If you use the socket functions in a truely MT'ed application,
   you have to protect the calls by locks. */

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

/* what is this for ?
#ifdef linux
int gethostname(char *name, unsigned int namelen);
#else
int gethostname(char *name, int namelen);
#endif
*/

typedef int           SOCKET;
#define INVALID_SOCKET  (SOCKET)(-1)
#define SOCKET_ERROR            (-1)

#define WSABASEERR              0 /* DUMMY */

#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

#define WSAEDISCON              (WSABASEERR+101)

#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)

#undef h_errno
#define h_errno         WSAGetLastError()

#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)

#define WSATRY_AGAIN            (WSABASEERR+1002)

#define WSANO_RECOVERY          (WSABASEERR+1003)

#define WSANO_DATA              (WSABASEERR+1004)

#define WSANO_ADDRESS            WSANO_DATA

#define WSA_IO_PENDING    4
#define WSA_IO_INCOMPLETE 4

#define closesocket close /* DUMMY */
/* int PASCAL FAR closesocket (SOCKET s); */

#define WSA_FLAG_OVERLAPPED 1
#define MSG_PARTIAL 2
#define WSA_INVALID_EVENT NULL
#define SD_SEND SHUT_WR
#define SD_BOTH SHUT_RDWR
#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef void* LPWSAPROTOCOL_INFO;

typedef int GROUP;

typedef HANDLE WSAEVENT;

typedef  void* LPWSAOVERLAPPED_COMPLETION_ROUTINE;

typedef struct _WSABUF{
  unsigned long  len;
  char FAR      *buf;
}WSABUF;

typedef struct WSAData {
        WORD                    wVersion;
        WORD                    wHighVersion;
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
} WSADATA;

typedef WSADATA FAR *LPWSADATA;

typedef WSABUF* LPWSABUF;

int PASCAL FAR WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);

int PASCAL FAR WSACleanup(void);

  /*SOCKET accept( SOCKET s, struct sockaddr FAR* addr, int FAR* addrlen);*/

typedef struct _WSAOVERLAPPED {
  WSAEVENT   hEvent;
  DWORD      BytesTransferred;
  BOOL       Completed;
}WSAOVERLAPPED;

typedef WSAOVERLAPPED OVERLAPPED;

typedef OVERLAPPED* LPOVERLAPPED;

typedef WSAOVERLAPPED* LPWSAOVERLAPPED;

typedef struct _Pair_MessageSocket
{
  LPWSABUF           Message;
  DWORD              NumberOfBytesSent;
  SOCKET             Socket;
  DWORD              BufferCount;
  DWORD              Flags;
  LPWSAOVERLAPPED    lpOverlapped;
}MessageSocket_t;

int PASCAL FAR ioctlsocket (SOCKET s, long cmd, u_long FAR *argp);

int PASCAL FAR WSASocket (int af,
			  int type,
			  int protocol,
			  LPWSAPROTOCOL_INFO lpProtocolInfo,
			  GROUP g,
			  DWORD dwFlags);

int PASCAL FAR WSASend(SOCKET s,
		       LPWSABUF lpBuffers,
		       DWORD dwBufferCount,
		       LPDWORD lpNumberOfBytesSent,
		       DWORD dwFlags,
		       LPWSAOVERLAPPED lpOverlapped,
		       LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

BOOL HasOverlappedIoCompleted (LPOVERLAPPED lpOverlapped);

BOOL WSAGetOverlappedResult (SOCKET s,
			     LPWSAOVERLAPPED lpOverlapped,
			     LPDWORD lpcbTransfer,
			     BOOL fWait,
			     LPDWORD lpdwFlags);

WSAEVENT WSACreateEvent(void);

BOOL WSACloseEvent(WSAEVENT hEvent);

int WSAEventSelect (SOCKET s, WSAEVENT hEventObject, long lNetworkEvent);

int WSARecv (SOCKET s,
             LPWSABUF lpBuffers,
	     DWORD dwBufferCount,
	     LPDWORD lpNumberOfBytesRecvd,
	     LPDWORD lpFlags,
	     LPWSAOVERLAPPED lpOverlapped,
	     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE);

int PASCAL FAR WSAGetLastError(void);
void PASCAL FAR WSASetLastError(int iError);

/* TO DO !!! *****

min


***** !!! TO DO */

/* not supported yet.
BOOL PASCAL FAR WSAIsBlocking(void);

int PASCAL FAR WSAUnhookBlockingHook(void);

FARPROC PASCAL FAR WSASetBlockingHook(FARPROC lpBlockFunc);

int PASCAL FAR WSACancelBlockingCall(void);
*/

#define TF_DISCONNECT       0x01
#define TF_REUSE_SOCKET     0x02
#define TF_WRITE_BEHIND     0x04

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr FAR *LPSOCKADDR;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in FAR *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger FAR *LPLINGER;

typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr FAR *LPIN_ADDR;

#ifdef sparc
typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set FAR *LPFD_SET;
#else
typedef  fd_set FD_SET;
typedef  fd_set *PFD_SET;
typedef  fd_set FAR *LPFD_SET;
#endif

typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent FAR *LPHOSTENT;

typedef struct servent SERVENT;
typedef struct servent *PSERVENT;
typedef struct servent FAR *LPSERVENT;

typedef struct protoent PROTOENT;
typedef struct protoent *PPROTOENT;
typedef struct protoent FAR *LPPROTOENT;

typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval FAR *LPTIMEVAL;

/* Exception Handling ----------------------------------------------- */
#define EXCEPTION_NONCONTINUABLE 0x1
#define EXCEPTION_MAXIMUM_PARAMETERS 15

typedef struct _EXCEPTION_RECORD {
  /*lint -e18 */  
  DWORD    ExceptionCode;
  /*lint +e18 */  
  DWORD ExceptionFlags;
  struct _EXCEPTION_RECORD *ExceptionRecord;
  PVOID ExceptionAddress;
  DWORD NumberParameters;
  DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD;
                                    
typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _CONTEXT {
    DWORDLONG FltF0;
    DWORDLONG FltF1;
    DWORDLONG FltF2;
    DWORDLONG FltF3;
    DWORDLONG FltF4;
    DWORDLONG FltF5;
    DWORDLONG FltF6;
    DWORDLONG FltF7;
    DWORDLONG FltF8;
    DWORDLONG FltF9;
    DWORDLONG FltF10;
    DWORDLONG FltF11;
    DWORDLONG FltF12;
    DWORDLONG FltF13;
    DWORDLONG FltF14;
    DWORDLONG FltF15;
    DWORDLONG FltF16;
    DWORDLONG FltF17;
    DWORDLONG FltF18;
    DWORDLONG FltF19;
    DWORDLONG FltF20;
    DWORDLONG FltF21;
    DWORDLONG FltF22;
    DWORDLONG FltF23;
    DWORDLONG FltF24;
    DWORDLONG FltF25;
    DWORDLONG FltF26;
    DWORDLONG FltF27;
    DWORDLONG FltF28;
    DWORDLONG FltF29;
    DWORDLONG FltF30;
    DWORDLONG FltF31;

    DWORDLONG IntV0;    
    DWORDLONG IntT0;    
    DWORDLONG IntT1;    
    DWORDLONG IntT2;    
    DWORDLONG IntT3;    
    DWORDLONG IntT4;    
    DWORDLONG IntT5;    
    DWORDLONG IntT6;    
    DWORDLONG IntT7;    
    DWORDLONG IntS0;    
    DWORDLONG IntS1;    
    DWORDLONG IntS2;    
    DWORDLONG IntS3;    
    DWORDLONG IntS4;    
    DWORDLONG IntS5;    
    DWORDLONG IntFp;    
    DWORDLONG IntA0;    
    DWORDLONG IntA1;    
    DWORDLONG IntA2;    
    DWORDLONG IntA3;    
    DWORDLONG IntA4;    
    DWORDLONG IntA5;    
    DWORDLONG IntT9;    
    DWORDLONG IntT10;   
    DWORDLONG IntT11;   
    DWORDLONG IntRa;    
    DWORDLONG IntT12;   
    DWORDLONG IntAt;    
    DWORDLONG IntGp;    
    DWORDLONG IntSp;    
    DWORDLONG IntZero;  

    DWORDLONG Fpcr;     
    DWORDLONG SoftFpcr; 

    DWORDLONG Fir;      
    DWORD Psr;          

    DWORD ContextFlags;
    DWORD Fill[4];      

} CONTEXT, *PCONTEXT;



typedef struct _EXCEPTION_POINTERS {
  PEXCEPTION_RECORD ExceptionRecord;
  PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

typedef PEXCEPTION_POINTERS LPEXCEPTION_POINTERS;

#define EXCEPTION_EXECUTE_HANDLER       1
#define EXCEPTION_CONTINUE_SEARCH       0
#define EXCEPTION_CONTINUE_EXECUTION    -1

#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)    
#define STATUS_ABANDONED_WAIT_0          ((DWORD   )0x00000080L)    
#define STATUS_USER_APC                  ((DWORD   )0x000000C0L)    
#define STATUS_TIMEOUT                   ((DWORD   )0x00000102L)    
#define STATUS_PENDING                   ((DWORD   )0x00000103L)    
#define STATUS_SEGMENT_NOTIFICATION      ((DWORD   )0x40000005L)    
#define STATUS_GUARD_PAGE_VIOLATION      ((DWORD   )0x80000001L)    
#define STATUS_DATATYPE_MISALIGNMENT     ((DWORD   )0x80000002L)    
#define STATUS_BREAKPOINT                ((DWORD   )0x80000003L)    
#define STATUS_SINGLE_STEP               ((DWORD   )0x80000004L)    
#define STATUS_ACCESS_VIOLATION          ((DWORD   )0xC0000005L)    
#define STATUS_IN_PAGE_ERROR             ((DWORD   )0xC0000006L)    
#define STATUS_NO_MEMORY                 ((DWORD   )0xC0000017L)    
#define STATUS_ILLEGAL_INSTRUCTION       ((DWORD   )0xC000001DL)    
#define STATUS_NONCONTINUABLE_EXCEPTION  ((DWORD   )0xC0000025L)    
#define STATUS_INVALID_DISPOSITION       ((DWORD   )0xC0000026L)    
#define STATUS_ARRAY_BOUNDS_EXCEEDED     ((DWORD   )0xC000008CL)    
#define STATUS_FLOAT_DENORMAL_OPERAND    ((DWORD   )0xC000008DL)    
#define STATUS_FLOAT_DIVIDE_BY_ZERO      ((DWORD   )0xC000008EL)    
#define STATUS_FLOAT_INEXACT_RESULT      ((DWORD   )0xC000008FL)    
#define STATUS_FLOAT_INVALID_OPERATION   ((DWORD   )0xC0000090L)    
#define STATUS_FLOAT_OVERFLOW            ((DWORD   )0xC0000091L)    
#define STATUS_FLOAT_STACK_CHECK         ((DWORD   )0xC0000092L)    
#define STATUS_FLOAT_UNDERFLOW           ((DWORD   )0xC0000093L)    
#define STATUS_INTEGER_DIVIDE_BY_ZERO    ((DWORD   )0xC0000094L)    
#define STATUS_INTEGER_OVERFLOW          ((DWORD   )0xC0000095L)    
#define STATUS_PRIVILEGED_INSTRUCTION    ((DWORD   )0xC0000096L)    
#define STATUS_STACK_OVERFLOW            ((DWORD   )0xC00000FDL)    
#define STATUS_CONTROL_C_EXIT            ((DWORD   )0xC000013AL)    

#define INVALID_HANDLE_VALUE (HANDLE)-1
#define INVALID_FILE_SIZE (DWORD)0xFFFFFFFF

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

#define TIME_ZONE_ID_INVALID (DWORD)0xFFFFFFFF

#define WAIT_FAILED (DWORD)0xFFFFFFFF
#define WAIT_OBJECT_0       ((STATUS_WAIT_0 ) + 0 )

#define WAIT_ABANDONED         ((STATUS_ABANDONED_WAIT_0 ) + 0 )
#define WAIT_ABANDONED_0       ((STATUS_ABANDONED_WAIT_0 ) + 0 )

#define WAIT_TIMEOUT                        STATUS_TIMEOUT
#define WAIT_IO_COMPLETION                  STATUS_USER_APC
#define STILL_ACTIVE                        STATUS_PENDING
#define EXCEPTION_ACCESS_VIOLATION          STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT     STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT                STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP               STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND      STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO        STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT        STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION     STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW              STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK           STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW             STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO        STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW              STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION          STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR             STATUS_IN_PAGE_ERROR
#define EXCEPTION_ILLEGAL_INSTRUCTION       STATUS_ILLEGAL_INSTRUCTION
#define EXCEPTION_NONCONTINUABLE_EXCEPTION  STATUS_NONCONTINUABLE_EXCEPTION
#define EXCEPTION_STACK_OVERFLOW            STATUS_STACK_OVERFLOW
#define EXCEPTION_INVALID_DISPOSITION       STATUS_INVALID_DISPOSITION
#define EXCEPTION_GUARD_PAGE                STATUS_GUARD_PAGE_VIOLATION
#define CONTROL_C_EXIT                      STATUS_CONTROL_C_EXIT
#define MoveMemory RtlMoveMemory
#define CopyMemory RtlCopyMemory
#define FillMemory RtlFillMemory
#define ZeroMemory RtlZeroMemory
#define RtlZeroMemory(a,b) memset(a,0,b)
#define RtlCopyMemory(a,b,c) memcpy(a,b,c)

WINBASEAPI
LONG
WINAPI
UnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

typedef LONG (WINAPI *PTOP_LEVEL_EXCEPTION_FILTER)(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

WINBASEAPI
LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    );
                                                                                        
LPEXCEPTION_POINTERS GetExceptionInformation(VOID);

#define PROCESS_TERMINATE         (0x0001)  
#define PROCESS_CREATE_THREAD     (0x0002)  
#define PROCESS_VM_OPERATION      (0x0008)  
#define PROCESS_VM_READ           (0x0010)  
#define PROCESS_VM_WRITE          (0x0020)  
#define PROCESS_DUP_HANDLE        (0x0040)  
#define PROCESS_CREATE_PROCESS    (0x0080)  
#define PROCESS_SET_QUOTA         (0x0100)  
#define PROCESS_SET_INFORMATION   (0x0200)  
#define PROCESS_QUERY_INFORMATION (0x0400)  
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)

#define THREAD_TERMINATE               (0x0001)  
#define THREAD_SUSPEND_RESUME          (0x0002)  
#define THREAD_GET_CONTEXT             (0x0008)  
#define THREAD_SET_CONTEXT             (0x0010)  
#define THREAD_SET_INFORMATION         (0x0020)  
#define THREAD_QUERY_INFORMATION       (0x0040)  
#define THREAD_SET_THREAD_TOKEN        (0x0080)
#define THREAD_IMPERSONATE             (0x0100)
#define THREAD_DIRECT_IMPERSONATION    (0x0200)

#undef CS /* This is defined under Solaris x86 */
   
#ifdef __cplusplus
}
#endif

#endif
