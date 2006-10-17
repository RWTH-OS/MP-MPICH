/*--------------------------------------------------------------------------*/
/*                                                                          */
/* PhD-Project                                                              */
/* (c) 1998 Martin Schulz                                                   */
/*                                                                          */
/* Global header file: errcodes.h                                           */
/* Defines the error codes and assertions for all modules                   */
/*                                                                          */
/* Headerfile (exported to all other modules)                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/


#ifndef _GLOB_ERRCODE
#define _GLOB_ERRCODE

/*--------------------------------------------------------------------------*/
/* error type definition */

typedef uint errCode_t;


/*--------------------------------------------------------------------------*/
/* Start-Assertions */

#ifdef ASSERT
#define ASSERT_START(var,mod,num) {char name[200];if (!(var)){sprintf(name,"ASSERTION failed, no %i: module not initialized !!!",num);_GLOB_InitErrorBox(mod,name);exit(1);}}
#else
#define ASSERT_START(var,mod,num)
#endif

/*--------------------------------------------------------------------------*/
/* Macros for the definition of the error codes  */

#define ALL_OK			0x00000000

#define ERRORMASK		0x80000000
#define SCIDIRECTMASK	        0x40000000
#define SISCIDIRECTMASK	        0x60000000

#define MAKE_ERROR(id,grpec)	         (ERRORMASK | (id << 20) | grpec)
#define MAKE_SCIDIRERR(ec)		 (ERRORMASK | SCIDIRECTMASK | ec)

#define CONVERT_SISCI_HAMSTER(sisci_Err) (ERRORMASK | SISCIDIRECTMASK | (sisci_Err))

/*--------------------------------------------------------------------------*/
/* Module IDs */

#define _ID_GLOBAL		0x01
#define	_ID_SCIAL		0x02
#define _ID_CLUSCTRL	        0x03
#define _ID_SCIVM		0x04
#define _ID_SIMPLESYNC	        0x05
#define _ID_SPMD		0x06
#define _ID_TMK			0x07
#define _ID_MYIOCTL             0x08
#define _ID_VMM                 0x09
#define _ID_DISTPTHREAD         0x0A


/*--------------------------------------------------------------------------*/
/* Error codes grouped */

#define MAKE_GRPERROR(grp,ec) ((grp << 12) | ec)


/*.......................................................................*/
/* System errors */

#define _GRP_OVERALL	0x01

#define _ERR_ALL_NOTSUPPORT		MAKE_GRPERROR(_GRP_OVERALL, 0x001)
#define _ERR_ALL_STARTUP                MAKE_GRPERROR(_GRP_OVERALL, 0x002)
#define _ERR_ALL_ALREADY                MAKE_GRPERROR(_GRP_OVERALL, 0x003)


/*.......................................................................*/
/* System errors */

#define _GRP_SYSTEM		0x02

#define _ERR_SYS_IOCTLFAIL		MAKE_GRPERROR(_GRP_SYSTEM, 0x001)
#define _ERR_SYS_MMAPFAIL		MAKE_GRPERROR(_GRP_SYSTEM, 0x002)
#define _ERR_SYS_FLUSHFAIL		MAKE_GRPERROR(_GRP_SYSTEM, 0x003)
#define _ERR_SYS_NOTCACHEABLE	MAKE_GRPERROR(_GRP_SYSTEM, 0x004)
#define _ERR_SYS_MAPERROR		MAKE_GRPERROR(_GRP_SYSTEM, 0x005)
#define _ERR_SYS_NODOSHEADER	MAKE_GRPERROR(_GRP_SYSTEM, 0x006)
#define _ERR_SYS_NONTHEADER		MAKE_GRPERROR(_GRP_SYSTEM, 0x007)
#define _ERR_SYS_LOCKFAIL		MAKE_GRPERROR(_GRP_SYSTEM, 0x008)
#define _ERR_SYS_UNLOCKFAIL		MAKE_GRPERROR(_GRP_SYSTEM, 0x009)
#define _ERR_SYS_PAGEFAIL		MAKE_GRPERROR(_GRP_SYSTEM, 0x00A)
#define _ERR_SYS_NOMEM			MAKE_GRPERROR(_GRP_SYSTEM, 0x00B)
#define _ERR_SYS_OPENFAIL       MAKE_GRPERROR(_GRP_SYSTEM, 0x00C)
#define _ERR_SYS_BFD            MAKE_GRPERROR(_GRP_SYSTEM, 0x00D)
#define _ERR_SYS_VIRTUNMAP      MAKE_GRPERROR(_GRP_SYSTEM, 0x00E)
#define _ERR_SYS_PROC           MAKE_GRPERROR(_GRP_SYSTEM, 0x00F)
#define _ERR_SYS_TLS            MAKE_GRPERROR(_GRP_SYSTEM, 0x010)
#define _ERR_SYS_NOGLOBALDATA   MAKE_GRPERROR(_GRP_SYSTEM, 0x011)


/*.......................................................................*/
/* Cluster errors */

#define _GRP_CLUSTER	0x03

#define _ERR_CLU_CONFIGFAIL		MAKE_GRPERROR(_GRP_CLUSTER, 0x001)
#define _ERR_CLU_HOSTNAME		MAKE_GRPERROR(_GRP_CLUSTER, 0x002)
#define _ERR_CLU_WRONGNUM		MAKE_GRPERROR(_GRP_CLUSTER, 0x003)
#define _ERR_CLU_SOCKOPT                MAKE_GRPERROR(_GRP_CLUSTER, 0x004)


/*.......................................................................*/
/* Message errors */

#define _GRP_MESSAGE	0x04

#define _ERR_MES_UNSUPTAG		MAKE_GRPERROR(_GRP_MESSAGE, 0x001)
#define _ERR_MES_WRONGTARGET	MAKE_GRPERROR(_GRP_MESSAGE, 0x003)
#define _ERR_MES_UNAVAILGRP		MAKE_GRPERROR(_GRP_MESSAGE, 0x004)
#define _ERR_MES_GRPNOTUSED		MAKE_GRPERROR(_GRP_MESSAGE, 0x005)


/*.......................................................................*/
/* Special TCP/IP errors */

#define _GRP_TCPIP		0x05

#define _ERR_TCP_SEND			MAKE_GRPERROR(_GRP_TCPIP, 0x001)
#define _ERR_TCP_RECEIVE		MAKE_GRPERROR(_GRP_TCPIP, 0x002)
#define _ERR_TCP_SOCKETINIT		MAKE_GRPERROR(_GRP_TCPIP, 0x003)
#define _ERR_TCP_CONNECT		MAKE_GRPERROR(_GRP_TCPIP, 0x004)
#define _ERR_TCP_LISTEN			MAKE_GRPERROR(_GRP_TCPIP, 0x005)
#define _ERR_TCP_BIND			MAKE_GRPERROR(_GRP_TCPIP, 0x006)
#define _ERR_TCP_ACCEPT			MAKE_GRPERROR(_GRP_TCPIP, 0x007)


/*.......................................................................*/
/* Errors from global operations */

#define _GRP_GLOBAL		0x06

#define _ERR_GLO_ZERO			MAKE_GRPERROR(_GRP_GLOBAL, 0x001)


/*.......................................................................*/
/* Errors from global operations */

#define _GRP_SCI		0x07

#define _ERR_SCI_OUTOFATT		MAKE_GRPERROR(_GRP_SCI, 0x001)
#define _ERR_SCI_TRANSMIT		MAKE_GRPERROR(_GRP_SCI, 0x002)


/*.......................................................................*/
/* Errors from IRM operations */

#define _GRP_IRM		0x08

#define _ERR_IRM_ATTSETFAIL		MAKE_GRPERROR(_GRP_IRM, 0x001)
#define _ERR_IRM_ATTALLOCFAIL	MAKE_GRPERROR(_GRP_IRM, 0x002)


/*.......................................................................*/
/* Errors from IRM operations */

#define _GRP_SISCI		0x09

#define _ERR_SISCI_CREATE		MAKE_GRPERROR(_GRP_SISCI,0x001)
#define _ERR_SISCI_PREPARE		MAKE_GRPERROR(_GRP_SISCI,0x002)
#define _ERR_SISCI_AVAIL		MAKE_GRPERROR(_GRP_SISCI,0x003)
#define _ERR_SISCI_CONNECT		MAKE_GRPERROR(_GRP_SISCI,0x004)
#define _ERR_SISCI_MAP			MAKE_GRPERROR(_GRP_SISCI,0x005)


/*.......................................................................*/
/* Errors from the runtime (resource problems) */

#define _GRP_RUNTIME	0x0A

#define _ERR_RUN_OUTOFVIRTMEM	MAKE_GRPERROR(_GRP_RUNTIME, 0x001)
#define _ERR_RUN_OUTOFPAGES		MAKE_GRPERROR(_GRP_RUNTIME, 0x002)
#define _ERR_RUN_OUTOFGLOBMEM	MAKE_GRPERROR(_GRP_RUNTIME, 0x003)
#define _ERR_RUN_OUTOFATOMIC	MAKE_GRPERROR(_GRP_RUNTIME, 0x004)
#define _ERR_RUN_OUTOFMEMORY	MAKE_GRPERROR(_GRP_RUNTIME, 0x005)


/*--------------------------------------------------------------------------*/
/* Error codes for the SCIAL module */

#define OK_SCIAL		ALL_OK

#define ERR_SCIAL_IOCTLFAIL			MAKE_ERROR(_ID_SCIAL, _ERR_SYS_IOCTLFAIL)
#define ERR_SCIAL_NOTSUPPORT		MAKE_ERROR(_ID_SCIAL, _ERR_ALL_NOTSUPPORT)
#define ERR_SCIAL_OUTOFATT			MAKE_ERROR(_ID_SCIAL, _ERR_SCI_OUTOFATT)
#define ERR_SCIAL_ATTSETFAIL		MAKE_ERROR(_ID_SCIAL, _ERR_IRM_ATTSETFAIL)
#define ERR_SCIAL_ATTALLOCFAIL		MAKE_ERROR(_ID_SCIAL, _ERR_IRM_ATTALLOCFAIL)
#define ERR_SCIAL_NOMEM				MAKE_ERROR(_ID_SCIAL, _ERR_SYS_NOMEM)
#define ERR_SCIAL_SISCICREATE		MAKE_ERROR(_ID_SCIAL, _ERR_SISCI_CREATE)
#define ERR_SCIAL_SISCIPREPARE		MAKE_ERROR(_ID_SCIAL, _ERR_SISCI_PREPARE)
#define ERR_SCIAL_SISCIAVAIL		MAKE_ERROR(_ID_SCIAL, _ERR_SISCI_AVAIL)
#define ERR_SCIAL_SISCICONNECT		MAKE_ERROR(_ID_SCIAL, _ERR_SISCI_CONNECT)
#define ERR_SCIAL_SISCIMAP			MAKE_ERROR(_ID_SCIAL, _ERR_SISCI_MAP)
#define ERR_SCIAL_SCIERROR			MAKE_ERROR(_ID_SCIAL, _ERR_SCI_TRANSMIT)
#define ERR_SCIAL_STARTUP               MAKE_ERROR(_ID_SCIAL, _ERR_ALL_STARTUP)


/*--------------------------------------------------------------------------*/
/* Error codes for the CLUSCTRL module */

#define OK_CLUSCTRL		ALL_OK

#define ERR_CLUSCTRL_CONFIGFAIL		MAKE_ERROR(_ID_CLUSCTRL, _ERR_CLU_CONFIGFAIL)
#define ERR_CLUSCTRL_UNSUPTAG		MAKE_ERROR(_ID_CLUSCTRL, _ERR_MES_UNSUPTAG)
#define ERR_CLUSCTRL_WRONGTARGET	MAKE_ERROR(_ID_CLUSCTRL, _ERR_MES_WRONGTARGET)
#define ERR_CLUSCTRL_UNAVAILGRP		MAKE_ERROR(_ID_CLUSCTRL, _ERR_MES_UNAVAILGRP)
#define ERR_CLUSCTRL_GRPNOTUSED		MAKE_ERROR(_ID_CLUSCTRL, _ERR_MES_GRPNOTUSED)
#define ERR_CLUSCTRL_SENDFAIL		MAKE_ERROR(_ID_CLUSCTRL, _ERR_TCP_SEND)
#define ERR_CLUSCTRL_RECVFAIL		MAKE_ERROR(_ID_CLUSCTRL, _ERR_TCP_RECEIVE)
#define ERR_CLUSCTRL_SOCKETFAIL		MAKE_ERROR(_ID_CLUSCTRL, _ERR_TCP_SOCKETINIT)
#define ERR_CLUSCTRL_CONNECTFAIL	MAKE_ERROR(_ID_CLUSCTRL, _ERR_TCP_CONNECT)
#define ERR_CLUSCTRL_LISTENFAIL		MAKE_ERROR(_ID_CLUSCTRL, _ERR_TCP_LISTEN)
#define ERR_CLUSCTRL_BINDFAIL		MAKE_ERROR(_ID_CLUSCTRL, _ERR_TCP_BIND)
#define ERR_CLUSCTRL_ACCEPTFAIL		MAKE_ERROR(_ID_CLUSCTRL, _ERR_TCP_ACCEPT)
#define ERR_CLUSCTRL_HOSTNAME		MAKE_ERROR(_ID_CLUSCTRL, _ERR_CLU_HOSTNAME)
#define ERR_CLUSCTRL_SOCKOPT		MAKE_ERROR(_ID_CLUSCTRL, _ERR_CLU_SOCKOPT)
#define ERR_CLUSCTRL_ZERO			MAKE_ERROR(_ID_CLUSCTRL, _ERR_GLO_ZERO)
#define ERR_CLUSCTRL_WRONGNUM		MAKE_ERROR(_ID_CLUSCTRL, _ERR_CLU_WRONGNUM)
#define ERR_CLUSCTRL_STARTUP               MAKE_ERROR(_ID_CLUSCTRL, _ERR_ALL_STARTUP)
#define ERR_CLUSCTRL_NOTSUP             MAKE_ERROR(_ID_CLUSCTRL, _ERR_ALL_NOTSUPPORT)
#define ERR_CLUSCTRL_PROCFAIL           MAKE_ERROR(_ID_CLUSCTRL, _ERR_SYS_PROC)


/*--------------------------------------------------------------------------*/
/* Error codes for the GLOBALID module */

#define OK_GLOBALID		ALL_OK

#define ERR_GLOBALID_STARTUP               MAKE_ERROR(_ID_GLOBALID, _ERR_ALL_STARTUP)


/*--------------------------------------------------------------------------*/
/* Error codes for the W32DTHREAD module */

#define OK_W32DTHREAD	ALL_OK


/*--------------------------------------------------------------------------*/
/* Error codes for the SCIVM module */

#define OK_SCIVM		ALL_OK

#define ERR_SCIVM_MMAPFAIL			MAKE_ERROR(_ID_SCIVM, _ERR_SYS_MMAPFAIL)
#define ERR_SCIVM_OUTOFMEMORY		MAKE_ERROR(_ID_SCIVM, _ERR_RUN_OUTOFMEMORY)
#define ERR_SCIVM_NOTCACHEABLE		MAKE_ERROR(_ID_SCIVM, _ERR_SYS_NOTCACHEABLE)
#define ERR_SCIVM_OUTOFVIRTMEM		MAKE_ERROR(_ID_SCIVM, _ERR_RUN_OUTOFVIRTMEM)
#define ERR_SCIVM_MAPERROR			MAKE_ERROR(_ID_SCIVM, _ERR_SYS_MAPERROR)
#define ERR_SCIVM_OUTOFPAGES		MAKE_ERROR(_ID_SCIVM, _ERR_RUN_OUTOFPAGES)
#define ERR_SCIVM_NODOSHEADER		MAKE_ERROR(_ID_SCIVM, _ERR_SYS_NODOSHEADER)
#define ERR_SCIVM_NONTHEADER		MAKE_ERROR(_ID_SCIVM, _ERR_SYS_NONTHEADER)
#define ERR_SCIVM_LOCKFAIL			MAKE_ERROR(_ID_SCIVM, _ERR_SYS_LOCKFAIL)
#define ERR_SCIVM_UNLOCKFAIL		MAKE_ERROR(_ID_SCIVM, _ERR_SYS_UNLOCKFAIL)
#define ERR_SCIVM_PAGEFAIL			MAKE_ERROR(_ID_SCIVM, _ERR_SYS_PAGEFAIL)
#define ERR_SCIVM_OUTOFGLOBMEM		MAKE_ERROR(_ID_SCIVM, _ERR_RUN_OUTOFGLOBMEM)
#define ERR_SCIVM_STARTUP               MAKE_ERROR(_ID_SCIVM, _ERR_ALL_STARTUP)
#define ERR_SCIVM_BFD                   MAKE_ERROR(_ID_SCIVM, _ERR_SYS_BFD)
#define ERR_SCIVM_VIRTUNMAPFAIL         MAKE_ERROR(_ID_SCIVM, _ERR_SYS_VIRTUNMAP)
#define ERR_SCIVM_NOTSUP			MAKE_ERROR(_ID_SCIVM, __ERR_ALL_NOTSUPPORT)
#define ERR_SCIVM_NOGLOBALDATA          MAKE_ERROR(_ID_SCIVM, _ERR_SYS_NOGLOBALDATA)

/*--------------------------------------------------------------------------*/
/* Error codes for the SIMPLESYNC module */

#define OK_SIMPLESYNC	ALL_OK

#define ERR_SIMPLESYNC_MMAPFAIL		MAKE_ERROR(_ID_SIMPLESYNC, _ERR_SYS_MMAPFAIL)
#define ERR_SIMPLESYNC_OUTOFATOMIC	MAKE_ERROR(_ID_SIMPLESYNC, _ERR_RUN_OUTOFATOMIC)
#define ERR_SIMPLESYNC_OUTOFMEMORY	MAKE_ERROR(_ID_SIMPLESYNC, _ERR_RUN_OUTOFMEMORY)
#define ERR_SIMPLESYNC_FLUSHFAIL	MAKE_ERROR(_ID_SIMPLESYNC, _ERR_SYS_FLUSHFAIL)
#define ERR_SIMPLESYNC_STARTUP          MAKE_ERROR(_ID_SIMPLESYNC, _ERR_ALL_STARTUP)
#define ERR_SIMPLESYNC_NOTSUP		MAKE_ERROR(_ID_SIMPLESYNC, __ERR_ALL_NOTSUPPORT)


/*--------------------------------------------------------------------------*/
/* Error codes for the SPMD module */

#define OK_SPMD			ALL_OK

#define ERR_SPMD_TLS				MAKE_ERROR(_ID_SPMD, _ERR_SYS_TLS)


/*--------------------------------------------------------------------------*/
/* Error codes for the TMK module */

#define OK_TMK			ALL_OK


/*--------------------------------------------------------------------------*/
/* Error codes for the MYIOCTL module */

#define OK_MYIOCTL		ALL_OK

#define ERR_MYIOCTL_IOCTLFAIL          MAKE_ERROR(_ID_MYIOCTL, _ERR_SYS_IOCTLFAIL)


/*--------------------------------------------------------------------------*/
/* Error codes for the DISTPTHREAD module */

#define OK_DISTPTHREAD		ALL_OK


/*--------------------------------------------------------------------------*/
/* Error codes for the VMM module */

#define OK_VMM		ALL_OK

#define ERR_VMM_OPEN          MAKE_ERROR(_ID_VMM, _ERR_SYS_OPENFAIL)
#define ERR_VMM_IOCTL         MAKE_ERROR(_ID_VMM, _ERR_SYS_IOCTLFAIL)
#define ERR_VMM_ALREADY       MAKE_ERROR(_ID_VMM, _ERR_ALL_ALREADY)
#define ERR_VMM_MMAP          MAKE_ERROR(_ID_VMM, _ERR_SYS_MMAPFAIL)


/*--------------------------------------------------------------------------*/
/* Text strings for all modules */

#define _TEXT_SCIAL			"SCI Abstraction Layer DLL"
#define _TEXT_CLUSCTRL		"Cluster Manager"
#define _TEXT_W32DTHREAD	"Distributed Win32 Threads"
#define _TEXT_GLOBALID		"Manager of global identifiers"
#define _TEXT_SCIVM			"SCI Virtual Memory"
#define _TEXT_SIMPLESYNC	"Simple Synchronization Library"
#define _TEXT_SPMD			"Single Program Multiple Data programming model"
#define _TEXT_TMK			"TreadMarks (r) compatibility library"
#define _TEXT_VMM               "VMM Driver library"
#define _TEXT_DISTPTHREAD       "Distributed Pthreads (SISCI-Pthreads)"


/*--------------------------------------------------------------------------*/
/* Error handling during initialization */

#ifdef WIN32
#define _GLOB_InitErrorBox(mod,et) MessageBox(NULL,et,mod,MB_OK | MB_ICONERROR);
#endif

#ifdef LINUX
#define _GLOB_InitErrorBox(mod,et) printf("\n!!!!!!!!!!!!!!!!!!!!!!!\nERROR in %s\n-> %s\nAborting\n!!!!!!!!!!!!!!!!!!!!!!!\n\n",mod,et);
#endif


/*--------------------------------------------------------------------------*/
/* The End. */

#endif
