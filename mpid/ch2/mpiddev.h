/* $Id$ */

/* the global device-related definitions */

#ifndef MPID_DEV_H
#define MPID_DEV_H

#include "dev.h"

/* Globals - for the devices */
extern int          MPID_n_pending;
extern MPID_DevSet *MPID_devset;
extern MPID_INFO   *MPID_tinfo;

/* function ptr to timing-function in a device */
extern double (*MPID_dev_wtime)(void);    

/* packing routines from adi2mpack.c */
extern int MPID_PackMessageFree (MPIR_SHANDLE *);
extern void MPID_PackMessage (void *, int, struct MPIR_DATATYPE *, 
					struct MPIR_COMMUNICATOR *, int, 
					MPID_Msgrep_t, MPID_Msg_pack_t, 
					void **, int *, int *);
extern void MPID_UnpackMessageSetup ( int, struct MPIR_DATATYPE *, 
						struct MPIR_COMMUNICATOR *,
						int, MPID_Msgrep_t, void **, 
						int *, int * );
extern int MPID_UnpackMessageComplete ( MPIR_RHANDLE * );

#endif
