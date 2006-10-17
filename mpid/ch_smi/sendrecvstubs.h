/* $Id: sendrecvstubs.h,v 1.4.8.1 2004/12/15 13:45:21 boris Exp $ */

#ifndef MPID_SMI_SEND_RECV_STUBS_H
#define MPID_SMI_SEND_RECV_STUBS_H

#include "adi3types.h"
#include "smidev.h"


void MPID_SMI_Init_sendrecv_stubs (void);
void MPID_SMIstub_SendContig (void*, int, int, int, int,
								int, MPID_Msgrep_t, int*);
void MPID_SMIstub_RecvContig (void*, int, int, int, int, 
								MPI_Status*, int*);
void MPID_SMIstub_IsendContig (	MPID_Comm*, void*, int, int, int, int,
								int, MPID_Msgrep_t, MPI_Request, int*);
void MPID_SMIstub_IrecvContig (	MPID_Comm*, void*, int, int, int, int,
								MPI_Request, int*);
void MPID_SMIstub_SendDatatype (MPID_Comm*, void*, int, MPID_Datatype*,
								int, int, int, int, int*);
void MPID_SMIstub_RecvDatatype (MPID_Comm*, void*, int, MPID_Datatype*,
								int, int, int, MPI_Status*, int*);
void MPID_SMIstub_IsendDatatype (	MPID_Comm*, void*, int, MPID_Datatype*,
									int, int, int, int, MPI_Request, int*);
void MPID_SMIstub_IrecvDatatype (	MPID_Comm*, void*, int, MPID_Datatype*,
									int, int, int, MPI_Request, int*);
void MPID_SMIstub_Iprobe (	MPID_Comm*, int, int, int, int*, int*,
							MPI_Status*);
void MPID_SMIstub_Pack (	void*, int, MPID_Datatype*, void*, int, int*,
							MPID_Comm*, int, MPID_Msgrep_t, MPID_Msg_pack_t,
							int*);
void MPID_SMIstub_Unpack (	void*, int, MPID_Msgrep_t, int*, void*, int,
							MPID_Datatype*, int*, MPID_Comm*, int, int*);
void MPID_SMIstub_PackMessage (	void*, int, MPID_Datatype*, MPID_Comm*, int,
								MPID_Msgrep_t, MPID_Msg_pack_t, void**, int*,
								int*);
void MPID_SMIstub_Pack_size (int, MPID_Datatype*, MPID_Msg_pack_t, int*);
int MPID_SMIstub_Barrier (MPID_Comm *);

#endif	/* MPID_SMI_SEND_RECV_STUBS_H */


/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:ts=0:sw=4:
 */
