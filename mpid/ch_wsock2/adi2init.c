/*
 *  $Id: adi2init.c,v 1.5 2001/08/14 14:51:48 joachim Exp $
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#define _WIN32_WINNT 0x0400
#include <wtypes.h>
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "patchlevel.h"
#include <stdio.h>
#include "../util/cmnargs.h"
#include "../util/queue.h"
#include "reqalloc.h"

#define BLOCK
#if defined (BLOCK)
#include "LogMpid.h"
extern HANDLE *MPID_Events;
#endif

/* Home for these globals */
int MPID_MyWorldSize, MPID_MyWorldRank;
int MPID_Print_queues = 0;
MPID_SBHeader MPIR_rhandles;
MPID_SBHeader MPIR_shandles;

/* This is a prototype for this function used to provide a debugger hook */
void *MPIR_Breakpoint ANSI_ARGS((void));

int mixed;

/***************************************************************************/
/* Some operations are completed in several stages.  To ensure that a      */
/* process does not exit from MPID_End while requests are pending, we keep */
/* track of how many are out-standing                                      */
/***************************************************************************/
int MPID_n_pending = 0;
/*
 * Create the MPID_DevSet device from the requested devices, and
 * initialize the device mapping
 */

/* This COULD be a single piece of permanent storage, but that is awkward
   for shared-memory versions (hot-spot-references).  */
MPID_DevSet *MPID_devset = 0;

extern MPID_Device *MPID_CH_InitMsgPass 
       ANSI_ARGS(( int *, char ***, int, int ));

extern unsigned long *Global2Local;
extern int MPID_numids;

int MPID_Complete_pending ANSI_ARGS((void));

static int MPID_Short_len = -1;
void MPID_Init( argc, argv, config, error_code )
int  *argc, *error_code;
void *config;
char ***argv;
{
    int i, np;
    MPID_Device *dev;
    MPID_Config *config_info = (MPID_Config *)config;

    /*
	* Create the device set structure.  Currently, only handles one
	* device and maps all operations to that device
	*/
    MPID_devset = (MPID_DevSet *)MALLOC( sizeof(MPID_DevSet) );
    if (!MPID_devset) {
		*error_code = MPI_ERR_INTERN;
		return ;
    }
    /* Make devset safe for initializations errors */
    MPID_devset->dev_list = 0;
	
	/* 
	* Get the device type and the number of processors
	*/ 
	dev = MPID_CH_InitMsgPass( argc, argv, MPID_Short_len, -1 );
	if (!dev) {
		*error_code = MPI_ERR_INTERN;
		return;
	}
	np = MPID_MyWorldSize;
	//MPID_devset->ndev = 2;
	MPID_devset->ndev = (dev->next?2:1);
	MPID_devset->dev  = (MPID_Device **)MALLOC( 
		np * sizeof(MPID_Device *) );
	if (!MPID_devset->dev) {
		*error_code = MPI_ERR_INTERN;
		return;
	}
	for (i = 0; i<np; i++) {
		if(Global2Local&&Global2Local[i]<MPID_numids && dev->next)
			MPID_devset->dev[i] = dev->next;
		else
			MPID_devset->dev[i] = dev;
	}
	
	//MPID_devset->ndev_list   = MPID_devset->ndev = 2;
	MPID_devset->ndev_list   = MPID_devset->ndev = (dev->next?2:1);
	MPID_devset->dev_list    = dev;
	mixed |= (MPID_devset->ndev>1);
	
#ifdef MPIR_MEMDEBUG
	MPID_trinit( MPID_MyWorldRank );
#endif
	/* 
	* Get the basic options.  Note that this must be AFTER the initialization
	* in case the initialization routine was responsible for sending the
	* arguments to other processors.
	*/
	MPID_ProcessArgs( argc, argv );
	
	MPID_Init_queue();
	
	/* Initialize the send/receive handle allocation system */
	/* Use the persistent version of send/receive (since we don't have
	separate MPIR_pshandles/MPIR_prhandles) */
	MPIR_shandles   = MPID_SBinit( sizeof( MPIR_PSHANDLE ), 100, 100 );
	MPIR_rhandles   = MPID_SBinit( sizeof( MPIR_PRHANDLE ), 100, 100 );
	/* Needs to be changed for persistent handles.  A common request form? */
	
	MPID_devset->req_pending = 0;
	*error_code = MPI_SUCCESS;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
void MPID_Abort( comm_ptr, code, user, str )
struct MPIR_COMMUNICATOR *comm_ptr;
int      code;
char     *user, *str;
{
    MPID_Device *dev;
    char abortString[256];

    fprintf( stderr, "[%d] %s Aborting program %s\n", MPID_MyWorldRank,
	     user ? user : "", str ? str : "!" );
    fflush( stderr );
    fflush( stdout );

    /* Also flag a debugger that an abort has happened so that it can take
     * control while there's still useful state to be examined.
     * Remember, MPIR_Breakpoint is a complete no-op unless the debugger
     * is present.
     */
    sprintf(abortString, "%s Aborting program %s", user ? user : "", 
	    str ? str : "!" );
    MPIR_debug_abort_string = abortString;
    MPIR_debug_state        = MPIR_DEBUG_ABORTING;
    MPIR_Breakpoint();

    /* We may be aborting before defining any devices */
    if (MPID_devset) {
	int found_dev = 0;
	dev = MPID_devset->dev_list;
	while (dev) {
	    found_dev = 1;
	    (*dev->abort)( comm_ptr, code, str );
	    dev = dev->next;
	}
	if (!found_dev) 
	    exit( code );
    }
    else {
	exit( code );
    }
}

void MPID_End()
{
    MPID_Device *dev, *ndev;

    DEBUG_PRINT_MSG("Entering MPID_End" )

    /* Finish off any pending transactions */
    /* Should this be part of the device terminate routines instead ? 
       Probably not, incase they need to be done in an arbitrary sequence */
    MPID_Complete_pending();
    
    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }

    /* Eventually make this optional */
    
    if (MPID_Print_queues)
	MPID_Dump_queues();

/* We should really generate an error or warning message if there 
   are uncompleted operations... */
    dev = MPID_devset->dev_list;
    while (dev) {
	ndev = dev->next;
	/* Each device should free any storage it is using INCLUDING dev */
	(*dev->terminate)( dev );
	dev = ndev;
    }

    /* Clean up request handles */
    MPID_SBdestroy( MPIR_shandles );
    MPID_SBdestroy( MPIR_rhandles );

/* Free remaining storage */
    FREE( MPID_devset->dev );
    FREE( MPID_devset );
#if defined(MPIR_MEMDEBUG) && defined(MPID_ONLY)
    /* MPI_Finalize also does this */
    MPID_trdump( stdout );
#endif
}

/* Returns 1 if something found, -1 otherwise (if is_blocking is 
   MPID_NOTBLOCKING)  */
int MPID_DeviceCheck( is_blocking )
MPID_BLOCKING_TYPE is_blocking;
{
    MPID_Device *dev;
    int found = 0;
    int lerr;
	int cnt=0;
	
    DEBUG_PRINT_MSG2( "Starting DeviceCheck for %d devices",MPID_devset->ndev_list)
		if (MPID_devset->ndev_list == 1) {
			dev = MPID_devset->dev_list;
			lerr = (*dev->check_device)( dev, is_blocking );
			return (lerr == 0) ? 1 : lerr;
		} else {
			while (!found) {
				dev = MPID_devset->dev_list;
				while (!found && dev) {
					lerr =  (*dev->check_device)( dev, MPID_NOTBLOCKING );
					found |= (lerr == 0);
					dev = dev->next;
				}
				if (is_blocking == MPID_NOTBLOCKING || found) {
					break;
				} 
				if(++cnt>80) {
					LOG_BLOCK(0);
					WaitForSingleObject(MPID_Events[MPID_MyWorldRank],1);
					LOG_BLOCK(1);
				} 
			}
		}
		DEBUG_PRINT_MSG( "Exiting DeviceCheck")
		return (found) ? 1 : -1;
}

int MPID_Complete_pending()
{
    MPID_Device *dev;
    int         lerr;

    DEBUG_PRINT_MSG( "Starting Complete_pending")
    if (MPID_devset->ndev_list == 1) {
	dev = MPID_devset->dev_list;
	while (MPID_n_pending > 0) {
	    lerr = (*dev->check_device)( dev, MPID_BLOCKING );
	    if (lerr > 0) {
		return lerr;
	    }
	}
    }
    else {
	while (MPID_n_pending > 0) {
	    dev = MPID_devset->dev_list;
	    while (dev) {
		lerr = (*dev->check_device)( dev, MPID_NOTBLOCKING );
		if (lerr > 0) {
		    return lerr;
		}
		dev = dev->next;
	    }
	}
    }
    DEBUG_PRINT_MSG( "Exiting Complete_pending")
    return MPI_SUCCESS;
}

void MPID_SetPktSize( len )
int len;
{
    MPID_Short_len = len;
}

int MPID_Collops_init( struct MPIR_COMMUNICATOR *comm, MPIR_COMM_TYPE comm_type)
{
    MPID_Device *dev;
    
    dev = MPID_devset->dev_list;
    while(dev) {
	if (dev->collops_init) {
	    dev->collops_init(comm, comm_type);
	}
	dev = dev->next;
    }
    
    return MPI_SUCCESS;
}

int MPID_CommInit( struct MPIR_COMMUNICATOR *oldcomm, 
		    struct MPIR_COMMUNICATOR *newcomm )
{
    /* If we have only one device, and this device offers a comm_init
       function, we call it. The behaviour for multipe devices is not 
       yet defined - should each device look at the new communicator 
       and see if it can do something with it? */
    if ((MPID_devset->ndev == 1) && MPID_devset->dev_list->comm_init) {
	MPID_devset->dev_list->comm_init(oldcomm, newcomm);
    } else {
	MPID_CH_Comm_msgrep( newcomm );
    }
    
    return MPI_SUCCESS;
}

int MPID_CommFree( struct MPIR_COMMUNICATOR *comm )
{
    if ((MPID_devset->ndev == 1) && MPID_devset->dev_list->comm_free) {
	MPID_devset->dev_list->comm_free(comm);
    }
    
    return MPI_SUCCESS;    
}

/*
  Perhaps this should be a util function
 */
int MPID_WaitForCompleteSend( request )
MPIR_SHANDLE *request;
{
    while (!request->is_complete)
	MPID_DeviceCheck( MPID_BLOCKING );
    return MPI_SUCCESS;
}

int MPID_WaitForCompleteRecv( request )
MPIR_RHANDLE *request;
{
    while (!request->is_complete)
	MPID_DeviceCheck( MPID_BLOCKING );
    return MPI_SUCCESS;
}


void MPID_Version_name( name )
char *name;
{
    MPID_Device *dev;
    char chdev_version[MPID_MAX_VERSION_NAME];
    
    sprintf(name, "ADI-2 layer version %s, ", ADI2PATCHLEVEL);
    
    /* determine the names of the transport facilities (the channel devices) */
    dev = MPID_devset->dev_list;
    while (dev) {
	if(dev->get_version) {
	    dev->get_version(chdev_version);
	    strncat (name, chdev_version, MPID_MAX_VERSION_NAME-strlen(name));
	}
	dev = dev->next;
    }

    return;
}

