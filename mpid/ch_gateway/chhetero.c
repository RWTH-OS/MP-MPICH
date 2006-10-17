/*
 *  $Id: chhetero.c,v 1.6 2004/04/08 11:40:44 boris Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
/* For MPID_ArgSqueeze */
#include "../util/cmnargs.h"
#include "mpimem.h"
#ifdef WIN32
#include <sys/types.h>
#include <winsock2.h>
#endif

/* #define DEBUG(a) {a} */
#define DEBUG(a)

#include "chhetero.h"

MPID_INFO *MPID_procinfo = 0;
MPID_H_TYPE MPID_byte_order;
#ifdef FOO
static char *(ByteOrderName[]) = { "None", "LSB", "MSB", "XDR" };
#endif
int MPID_IS_HETERO = 0;

/* Local definitions */
int MPID_GetByteOrder ANSI_ARGS((void));
void MPID_ByteSwapInt ANSI_ARGS(( int*, int ));

/* 
 * This routine is called to initialize the information about datatype
 * representation at other processors.
 */
int MPID_CH_Init_hetero( argc, argv )
int  *argc;
char ***argv;
{
    int  i, use_xdr;
    char *work;

/* This checks for word sizes, so that systems
   with 32 bit ints can interoperate with 64 bit ints, etc. 
   We can check just the basic signed types: short, int, long, float, double 
   Eventually, we should probably also check the long long and long double
   types.
   We do this by collecting an array of word sizes, with each processor
   contributing the 5 lengths.  This is combined with the "byte order"
   field.

   We still need to identify IEEE and non-IEEE systems.  Perhaps we'll
   just use a field from configure (only CRAY vector systems are non IEEE
   of the systems we interact with).

   We ALSO need to identify Fortran sizes, at least sizeof(REAL), 
   and change the heterogenous code to handle Fortran LOGICALs
 */
/*
   We look for the argument -mpixdr to force use of XDR for debugging and
   timing comparision.
 */
    DEBUG_PRINT_MSG("Checking for heterogeneous systems...");
    MPID_procinfo = (MPID_INFO *)MALLOC( MPID_MyAllSize * sizeof(MPID_INFO) );
    if (!MPID_procinfo) {
	return MPI_ERR_INTERN;
    }
    for (i=0; i<MPID_MyAllSize; i++) {
	MPID_procinfo[i].byte_order	  = MPID_H_NONE;
	MPID_procinfo[i].short_size	  = 0;
	MPID_procinfo[i].int_size	  = 0;
	MPID_procinfo[i].long_size	  = 0;
	MPID_procinfo[i].float_size	  = 0;
	MPID_procinfo[i].double_size	  = 0;
	MPID_procinfo[i].long_double_size = 0;
	MPID_procinfo[i].float_type	  = 0;
    }
/* Set my byte ordering and convert if necessary.  */

/* Set the floating point type.  IEEE is 0, Cray is 2, others as we add them
   (MPID_FLOAT_TYPE?) Not yet set: VAX floating point,
   IBM 360/370 floating point, and other. */
#ifdef MPID_FLOAT_CRAY
    MPID_procinfo[MPID_MyAllRank].float_type = 2;
#endif
    use_xdr = 0;
    /* PRINTF ("Checking args for -mpixdr\n" ); */
    for (i=1; i<*argc; i++) {
	/* PRINTF( "Arg[%d] is %s\n", i, (*argv)[i] ? (*argv)[i] : "<NULL>" ); */
	if ((*argv)[i] && strcmp( (*argv)[i], "-mpixdr" ) == 0) {
	    /* PRINTF( "Found -mpixdr\n" ); */
	    use_xdr = 1;
	    (*argv)[i] = 0;
	    MPID_ArgSqueeze( argc, *argv );
	    break;
	}
    }

    if (use_xdr) 
	MPID_byte_order = MPID_H_XDR;
    else {
	i = MPID_GetByteOrder( );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
	DEBUG(FPRINTF(MPID_DEBUG_FILE,"[%d] Byte order is %d\n",MPID_MyAllRank, i ););
#endif                  /* #DEBUG_END# */
	if (i == 1)      MPID_byte_order = MPID_H_LSB;
	else if (i == 2) MPID_byte_order = MPID_H_MSB;
	else             MPID_byte_order = MPID_H_XDR;
    }
    MPID_procinfo[MPID_MyAllRank].byte_order	     = MPID_byte_order;
    MPID_procinfo[MPID_MyAllRank].short_size	     = sizeof(short);
    MPID_procinfo[MPID_MyAllRank].int_size	     = sizeof(int);
    MPID_procinfo[MPID_MyAllRank].long_size	     = sizeof(long);
    MPID_procinfo[MPID_MyAllRank].float_size	     = sizeof(float);
    MPID_procinfo[MPID_MyAllRank].double_size	     = sizeof(double);
#if defined(HAVE_LONG_DOUBLE)
    MPID_procinfo[MPID_MyAllRank].long_double_size = sizeof(long double);
    /* Otherwise leave as zero */
#endif

/* Everyone uses the same format (MSB) */
/* This should use network byte order OR the native collective operation 
   with heterogeneous support */
/* if (i == 1) 
   MPID_ByteSwapInt( (int*)&MPID_procinfo[MPID_MyAllRank].byte_order, 1 );
 */
    /* for MetaMPICH, MPID_IS_HETERO is set according to the option
       in the config file */
#ifdef META
    MPID_IS_HETERO = MPIR_meta_cfg.is_hetero;
#else
/* Get everyone else's */
    work = (char *)MALLOC( MPID_MyAllSize * sizeof(MPID_INFO) );
    if (!work) 
	return MPI_ERR_INTERN;
/* ASSUMES MPID_INFO is ints */
    /* XXX what the hell is PIgimax() ? */
    PIgimax( MPID_procinfo, 
	     (sizeof(MPID_INFO)/sizeof(int)) * MPID_MyAllSize, 
	     work, PSAllProcs );
    FREE( work );

/* See if they are all the same and different from XDR*/
    MPID_IS_HETERO = MPID_procinfo[0].byte_order == MPID_H_XDR;
    for (i=1; i<MPID_MyAllSize; i++) {
	if (MPID_procinfo[0].byte_order  != MPID_procinfo[i].byte_order ||
	    MPID_procinfo[i].byte_order  == MPID_H_XDR ||
	    MPID_procinfo[0].short_size  != MPID_procinfo[i].short_size ||
	    MPID_procinfo[0].int_size    != MPID_procinfo[i].int_size ||
	    MPID_procinfo[0].long_size   != MPID_procinfo[i].long_size ||
	    MPID_procinfo[0].float_size  != MPID_procinfo[i].float_size ||
	    MPID_procinfo[0].double_size != MPID_procinfo[i].double_size ||
	    MPID_procinfo[0].long_double_size != 
	    MPID_procinfo[i].long_double_size ||
	    MPID_procinfo[0].float_type  != MPID_procinfo[i].float_type) {
	    MPID_IS_HETERO = 1;
	    break;
        }
    }
#endif
/* 
   When deciding to use XDR, we need to check for size as well (if 
   [myid].xxx_size != [j].xxx_size, set [j].byte_order = XDR).  Note that 
   this is reflexive; if j decides that i needs XDR, i will also decide that
   j needs XDR;

   Note that checking for long double is also sort of strange; original
   XDR is pre-ANSI C and has no long double conversion.  So checking for
   this only causes us to fail more deterministically.  
 */
    if (MPID_IS_HETERO) {
	for (i=0; i<MPID_MyAllSize; i++) {
	    if (i == MPID_MyAllRank) continue;
#ifdef META
	    /* for MetaMPICH, use XDR for every process running
	       on another host or coming from a router, but 
	       no encoding for processes on the same host */
	    /* XXX this should be optimized to use XDR only when
	       really necessary */
	    if ((i < MPIR_meta_cfg.metahost_firstrank) 
		|| (i >= MPIR_meta_cfg.metahost_firstrank + MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank] + MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank] )) {
		    MPID_procinfo[i].byte_order = MPID_H_XDR;
	    }
	    else {
		memcpy ((void *)&MPID_procinfo[i], (void *)&MPID_procinfo[MPID_MyAllRank], sizeof(MPID_INFO));
	    }
#else
	    if (MPID_procinfo[MPID_MyAllRank].short_size  != 
		MPID_procinfo[i].short_size ||
		MPID_procinfo[MPID_MyAllRank].int_size    != 
		MPID_procinfo[i].int_size ||
		MPID_procinfo[MPID_MyAllRank].long_size   != 
		MPID_procinfo[i].long_size ||
		MPID_procinfo[MPID_MyAllRank].float_size  != 
		MPID_procinfo[i].float_size ||
		MPID_procinfo[MPID_MyAllRank].double_size != 
		MPID_procinfo[i].double_size ||
		MPID_procinfo[MPID_MyAllRank].long_double_size != 
		MPID_procinfo[i].long_double_size ||
		MPID_procinfo[MPID_MyAllRank].float_type !=
		MPID_procinfo[i].float_type) {
		MPID_procinfo[i].byte_order = MPID_H_XDR;
	    }
#endif
	}
    }
#ifdef FOO
    if (MPID_IS_HETERO && MPID_MyAllRank == 0) {
	printf( "Warning: heterogenity only partially supported\n" );
	printf( 
	"Ordering short int long float double longdouble sizes float-type\n" );
	for (i=0; i<MPID_MyAllSize; i++) {
	    printf( "<%d> %s %d %d %d %d %d %d %d\n", i,
		    ByteOrderName[MPID_procinfo[i].byte_order],
		    MPID_procinfo[i].short_size, 
		    MPID_procinfo[i].int_size, 
		    MPID_procinfo[i].long_size, 
		    MPID_procinfo[i].float_size, 
		    MPID_procinfo[i].double_size,
		    MPID_procinfo[i].long_double_size,
		    MPID_procinfo[i].float_type );
	}
    }
#endif
    return MPI_SUCCESS;
}

#ifdef FOO
/* 
 * Note that this requires an ABSOLUTE destination; ANY or RELATIVE 
 * are not valid.
 */
int MPID_CH_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}
#endif

#ifdef MPID_HAS_HETERO
/*
 * This routine takes a communicator and determines the message representation
 * field for it
 */
int MPID_CH_Comm_msgrep( comm_ptr )
struct MPIR_COMMUNICATOR *comm_ptr;
{
    MPID_H_TYPE my_byte_order;
    int i;

/* We must compare the rep of the rank in the local group to
   the members of the remote group.  This works for both intra and inter 
   communicators. 
   
   Note that in the intracommunicator case, we COULD use a receiver rep, if
   all receivers had the same representation.  The current code assumes
   that the sender is a potential receiver, so it can't use receiver order.
*/
    if (!MPID_IS_HETERO) {
	comm_ptr->msgform = MPID_MSG_OK;
	return MPI_SUCCESS;
    }
    
    my_byte_order = MPID_procinfo[MPID_MyAllRank].byte_order;

    if (my_byte_order == MPID_H_XDR) {
	comm_ptr->msgform = MPID_MSG_XDR;
	return MPI_SUCCESS;
    }
  
    /* This uses the "cached" attributes; this also allows an 
       implementation to use a simplified communicator */
    for (i = 0; i < comm_ptr->np; i++) {
	if (MPID_procinfo[comm_ptr->lrank_to_grank[i]].byte_order !=
	    my_byte_order) {
	    comm_ptr->msgform = MPID_MSG_XDR;
	    return MPI_SUCCESS;
	}
    }
    
/* receiver is == 0, so this says "no change" (sender and receiver have
   same format).  This needs to change... */
    comm_ptr->msgform = MPID_MSG_OK;
    return MPI_SUCCESS;
}
#endif

/* This routine is ensure that the elements in the packet HEADER can
   be read by the receiver without further processing (unless XDR is
   used, in which case we use network byte order)
   This routine is defined ONLY for heterogeneous systems

   Note that different packets have different lengths and layouts; 
   this makes the conversion more troublesome.  I'm still thinking
   about how to do this.
 */
#include <sys/types.h>
#ifndef WIN32
#include <netinet/in.h>
#endif
/* These need to use 32bit ints.  The 4's here are sizeof(int32) */

void MPID_CH_Pkt_pack( in_pkt, size, dest )
void       *in_pkt;
int        size, dest;
{
MPID_PKT_T *pkt = (MPID_PKT_T *)in_pkt;
int i;
unsigned int *d;
if (MPID_IS_HETERO &&
    (MPID_procinfo[dest].byte_order != MPID_byte_order ||
     MPID_byte_order == MPID_H_XDR)) {
    
    if (MPID_procinfo[dest].byte_order == MPID_H_XDR ||
	MPID_byte_order == MPID_H_XDR) {
	d = (unsigned int *)pkt;
	for (i=0; i<size/4; i++) {
	    *d = htonl(*d);
	    d++;
	    }
	}
    else {
	/* Need to swap to receiver's order.  We ALWAYS reorder at the
	   sender's end (this is because a message can be received with 
	   MPI_Recv instead of MPI_Recv/MPI_Unpack, and hence requires us 
	   to use a format that matches the receiver's ordering without 
	   requiring a user-unpack.  */
	MPID_ByteSwapInt( (int*)pkt, size / 4 );
	}
    }
}

void MPID_CH_Pkt_unpack( in_pkt, size, from )
void       *in_pkt;
int        size, from;
{
MPID_PKT_T *pkt = (MPID_PKT_T *)in_pkt;
int i;
unsigned int *d;

if (MPID_IS_HETERO &&
    (MPID_procinfo[from].byte_order != MPID_byte_order ||
     MPID_byte_order == MPID_H_XDR)) {
    
    if (MPID_procinfo[from].byte_order == MPID_H_XDR ||
	MPID_byte_order == MPID_H_XDR) {
	d = (unsigned int *)pkt;
	for (i=0; i<size/4; i++) {
	    *d = ntohl(*d);
	    d++;
	    }
	}
    }
}

int MPID_CH_Hetero_free()
{
    if (MPID_procinfo) 
	FREE( MPID_procinfo );
    return MPI_SUCCESS;
}

int MPID_GetByteOrder( )
{
    int l = 1;
    char *b = (char *)&l;

    if (b[0] == 1) return 1;
    if (b[sizeof(int)-1] == 1) return 2;
    return 0;
}

void MPID_ByteSwapInt(buff,n)
int *buff,n;
{
    int  i,j,tmp;
    char *ptr1,*ptr2 = (char *) &tmp;

    for ( j=0; j<n; j++ ) {
	ptr1 = (char *) (&buff[j]);
	for (i=0; i<sizeof(int); i++) {
	    ptr2[i] = ptr1[sizeof(int)-1-i];
	}
	buff[j] = tmp;
    }
}
