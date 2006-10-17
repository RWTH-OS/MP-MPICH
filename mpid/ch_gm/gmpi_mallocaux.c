/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifdef WIN32
/* Windows doesn't use the aux funtions. */
int foo;
#else
#include <unistd.h>
#include <unistd.h>
#include <sys/mman.h>

#include "gmpi.h"

#if !defined(__linux__) && !defined(__APPLE__)
/* loic: at least on 64bits archs, it is important that sbrk
   has the right prototype sbrk altough quite UNIX universal
   is a non-official function, so might not be in headers
   if this definition conflicts with yours, remove the OS
   from the def condition
 */
void *sbrk();
#endif

#if GM_OS_LINUX
#include <syscall.h>

int munmap(void *start, size_t length)
{
  gmpi.malloc_hook_flag = 1;

  GMPI_DEBUG_REG_CACHE_PRINT2("munmap stub", "start", start, "length", length);
  GMPI_DEBUG_REGISTRATION_CHECK_ALIGN("munmap (stub)", (gm_up_t)start, length);
  gmpi_clear_interval((gm_up_t)start, length);
  return syscall(__NR_munmap, start, length);
}

int __munmap(void *start, size_t length)
{
  return munmap(start, length);
}

#endif


void *gmpi_sbrk(int inc)
{
  gmpi.malloc_hook_flag = 1;
      
#if (GM_OS_SOLARIS || GM_OS_SOLARIS7 || GM_OS_MACOSX)
  return (-1);
#else
  if (inc < 0)
    {
      long oldp = (long)sbrk(0);
      
      GMPI_DEBUG_REG_CACHE_PRINT2("gmpi_sbrk", "oldp", oldp, "inc", inc);
      GMPI_DEBUG_REGISTRATION_CHECK_ALIGN("gmpi_sbrk", 
					  (gm_up_t)(oldp+inc), (-inc));
      gmpi_clear_interval((gm_up_t)(oldp+inc), -inc);
    }
  return sbrk(inc);
#endif
}


int gmpi_munmap(void *start, size_t length)
{
  gmpi.malloc_hook_flag = 1;

  GMPI_DEBUG_REG_CACHE_PRINT2("gmpi_munmap", "start", start, "length", length);
  GMPI_DEBUG_REGISTRATION_CHECK_ALIGN("gmpi_munmap", (gm_up_t) start, length);
  gmpi_clear_interval((gm_up_t)start, length);
  return munmap(start, length);
}

#endif
