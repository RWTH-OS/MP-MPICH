/*--------------------------------------------------------------------------*/
/*                                                                          */
/* User-level Module: SYNCMOD                                               */
/*                                                                          */
/* (c) 1998-2001 Martin Schulz, LRR-TUM                                     */
/*                                                                          */
/* Contains the HAMSTER routines for synchronization                        */
/* Standalone SISCI Version                                                 */
/*                                                                          */
/* Main file for SyncMod based on HAMSTER synchronization module            */
/*   Used as a general framework for the SyncMod                            */
/*   Includes the actual functionality from other files                     */
/*   files located in global or include are directly taken from HAMSTER     */
/*                                                                          */
/*--------------------------------------------------------------------------*/


#include "sync.h"
#ifdef USE_SCHULZSYNC

#define _IMPL_SIMPLESYNC
#define _IMPL_DLL

#define SIMPLESYNC_ASSERT(num) ASSERT_START(simpleSync_started,_TEXT_SIMPLESYNC,num);


#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <winbase.h>
#include <winnt.h>
#include <winioctl.h>
#include "u:\sisci\vmm-drv\testappl\vmmdll.h"
#endif


/* this is a patch, to be removed lateron */
#ifdef _REENTRANT
#undef _REENTRANT
#include "sisci_api.h"
#define _REENTRANT
#else
#include "sisci_api.h"
#endif /* _REENTRANT */
#ifndef LINUX
#define LINUX
#endif 

#ifdef USED_WITHIN_SMI
#include <proper_shutdown/sisci_resource.h>
#endif

#ifdef LINUX
#include <string.h>
#include <pthread.h>
#endif

#include "syncmod.h"


/*--------------------------------------------------------------------------*/
/* Load emulation defintions to compensate for missing HAMSTER environment */

#include "syncmod/emul.h"


/*--------------------------------------------------------------------------*/
/* Global structurs */

#include "syncmod/syncvars.c"


/*--------------------------------------------------------------------------*/
/* Library initialization and cleanup */

#include "syncmod/syncinit.c"


/*--------------------------------------------------------------------------*/
/* Basic functionality to manage the atomic segment */

#include "syncmod/syncatom.c"


/*--------------------------------------------------------------------------*/
/* Basic routines to perform synchronization */

#include "syncmod/synccore.c"


/*--------------------------------------------------------------------------*/
/* Public routines */

#include "syncmod/syncapi.c"


/*--------------------------------------------------------------------------*/
/* New public routines for identifier distribution */

#include "syncmod/syncdist.c"


/*--------------------------------------------------------------------------*/
/* The End. */

#else /* USE_SCHULZSYNC */
int xxx_dummy_xxx;
#endif /* USE_SCHULZSYNC */
