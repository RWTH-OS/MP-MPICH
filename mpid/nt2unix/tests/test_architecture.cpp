/* $id$ */
/* GetSystemInfo test */

#include <windows.h>

#if 1
#ifndef WIN32
#include "nt2unix.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef __SPARC
#include <sys/systeminfo.h>
#endif

#include <sys/sysinfo.h>
#include <sys/utsname.h>
#endif 

#include <windows.h>

extern char _etext;
extern char _edata;
extern char _end;

#define BUFLEN 80

int main (int argc, char ** argv)
{
  struct rlimit rlp;
  char buf[BUFLEN]; 
  struct utsname utn; 
  SYSTEM_INFO s; 
  DWORD *p = 0; 
  
  printf("_etext == %x\n", &_etext);
  printf("_edata == %x\n", &_edata);
  printf("_end   == %x\n", &_end);
  printf("sbrk == %x\n", sbrk(0));

  if (getrlimit(RLIMIT_DATA, &rlp))
    perror("getrlimit()");
  printf("_etext + RLIMIT_DATA == %x\n", ((DWORD)&_etext) + rlp.rlim_max);  
 
  if(getrlimit(RLIMIT_STACK, &rlp))
    perror("getrlimit()");
  printf("0xFFFFFFFF - RLIMIT_STACK == %x\n", 0xFFFFFFFF-rlp.rlim_max);  
  
  if (uname( &utn)) {
  	printf("error calling utsname\n");	
	exit(0);
  }
  printf("%s \n", utn.machine);

#ifdef __SPARC  
  sysinfo(SI_ARCHITECTURE , buf, BUFLEN);
  printf("%s \n", buf);
#endif
  
  GetSystemInfo(&s);
  printf("wProcessorArchitecure == %d\n", s.wProcessorArchitecture);
  printf("dwNumberOfProcessors  == %d\n", s.dwNumberOfProcessors);
  printf("dwActiveProcessorMask == %d\n", s.dwActiveProcessorMask);
  
  p = (DWORD*)VirtualAlloc(0, 0x0001, 0, PAGE_READWRITE);
  *p = 0;  
  printf("sbrk == %x\n", sbrk(0));
}
