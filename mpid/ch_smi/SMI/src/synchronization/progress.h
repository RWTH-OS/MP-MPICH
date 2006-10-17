/* $Id: progress.h,v 1.1 2004/03/19 22:14:23 joachim Exp $ */

/*********************************************************************************/
/*********************************************************************************/
/***                                                                           ***/
/*** This module implements progress counter as a mechanism of synchronization.***/
/*** A progress counter (PC) lets processes express their own progress in some ***/
/*** computation process to others. It has a "at least" interpretation, i.e.   ***/
/*** a progress value of another process means that this reached at least the  ***/
/*** seen progress but is maybe already ahead of this state. There are several ***/
/*** functions to enforce a process to wait until others have made at least a  ***/
/*** specified progress.                                                       ***/
/***                                                                           ***/
/*********************************************************************************/
/*********************************************************************************/

#ifndef _PROGRESS_H_
#define _PROGRESS_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************/
/* Set-up a new progress counter. It's id is returned. This is a collective call!*/
/* setting is_active to FALSE makes the calling process inactive in collective   */
/* synchronisation, you can define a set of synchronized processes this way      */
/* Possible errors: SMI_ERR_NOINIT and SMI_ERR_NOMEM                             */
/*********************************************************************************/
smi_error_t SMI_Init_PC(int* id);


/*********************************************************************************/
/* Reset the specified pc to zero. This is a collective function call !          */
/* possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id wrong)                     */
/*********************************************************************************/
smi_error_t SMI_Reset_PC(int id);


/*********************************************************************************/
/* Increment the own part of the specified pc by a provided quantity. This is an */
/* individual call. Possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id wrong).   */
/*********************************************************************************/
smi_error_t SMI_Increment_PC(int id, int val);


/*********************************************************************************/
/* Get a pc-counter vaule (returned in pc_val) of the specified pc and the       */
/* specified process. This is an individual call. Possible errors:               */
/* SMI_ERR_NOINIT, SMI_ERR_PARAM (pcid or proc_id wrong).                        */
/*********************************************************************************/
smi_error_t SMI_Get_PC(int pcid, int proc_id, int* pc_val);


/*********************************************************************************/
/* Waiting untill the progress counter of a specified pc (id) and of a specified */
/* process reaches a specified value. If val=SMI_OWNPC is specified, it is waited*/
/* until the own pc values has been reached. This is a individual call. Possible */
/* errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id or proc_rank wrong).                */
/*********************************************************************************/
smi_error_t SMI_Wait_individual_PC(int id, int proc_rank, int val);


/*********************************************************************************/
/* Waiting untill the progress counter of a specified pc (id) and of a set of    */
/* specified processes reaches a specified value. If val=SMI_OWNPC is specified, */
/* it is waited until the own pc values has been reached. This is a individual   */
/* call. Possible errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id or proc_rank wrong)  */
/*********************************************************************************/
smi_error_t SMI_Wait_individual_set_PC(int id, int* proc_ranks, int nbr_ranks, int val);


/*********************************************************************************/
/* Waiting untill the progress counters of a specified pc (id) and of all        */
/* processs reach a specified value. If val=SMI_OWNPC is specified, it is waited */
/* until the own pc values have been reached. This is a individual call. Possible*/
/* errors: SMI_ERR_NOINIT, SMI_ERR_PARAM (id or proc_rank wrong).                */
/*********************************************************************************/
smi_error_t SMI_Wait_collective_PC(int id, int val);

#ifdef __cplusplus
}
#endif


#endif

