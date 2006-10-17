/* $Id$ */

#ifndef _BARRIER_H_
#define _BARRIER_H_

#include "sync.h"
#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif



/*********************************************************************************/
/*** Function Headers                                                          ***/
/*********************************************************************************/

void _smi_barrier_module_init(void);
void _smi_barrier_module_finalize(void);

/*************************************/
/* Error-Codes: SMI_ERR_NOINIT       */
/*              SMI_ERR_PARAM        */
/*      		Systemerrors         */
/*              MPI Errors	         */
/* and errors form all barrier inits */
/*************************************/
smi_error_t SMI_BARRIER_INIT(int*, int, int);

/***************************************/
/* Error-Codes: SMI_ERR_NOINIT         */
/*              SMI_ERR_PARAM          */
/* and errors from all barrier destroy */
/***************************************/
smi_error_t SMI_BARRIER_DESTROY(int);

/*******************************/
/* Error-Codes: SMI_ERR_NOINIT */
/*              SMI_ERR_PARAM  */
/*******************************/
smi_error_t SMI_BARRIER(int);


/*********************************************************************************/
/*** This is a barrier, based on the progress counters with local spinning. It ***/
/*** should be the most efficient and the least critical regarding correctness.***/
/*********************************************************************************/
smi_error_t SMI_ProgressCounterBarrier_init(barrier_id*);
smi_error_t SMI_ProgressCounterBarrier_destroy(barrier_id*);
smi_error_t SMI_ProgressCounterBarrier(barrier_id*);

/*********************************************************************************/
/*** This is a barrier, based on the implementations of Martin Schulz wich are ***/
/*** directly based on dolphins sisci                                          ***/
/*********************************************************************************/
smi_error_t SMI_SchulzBarrier_init(barrier_id* ID);
smi_error_t SMI_SchulzBarrier(barrier_id* ID);
smi_error_t SMI_SchulzBarrier_destroy(barrier_id* ID);

#ifdef __cplusplus
}
#endif


#endif
