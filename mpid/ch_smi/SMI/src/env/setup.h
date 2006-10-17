/* $Id$ */


/*****************************************************************************/
/*****************************************************************************/
/*** This module provides the SMI with all necessary information regarding ***/
/*** the hardware/software environment on which the ensemble of processes  ***/
/*** are actually executed.                                                ***/
/*****************************************************************************/
/*****************************************************************************/


#ifndef __SETUP_H
#define __SETUP_H


#include "general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************/
/*** Determine the process_ids of all processes                               ***/
/********************************************************************************/
smi_error_t _smi_get_pids(void);

/*****************************************************************************/
/*** Determines the total number of processes and stores it in the global  ***/
/*** variable 'no_processes'.                                              ***/
/*** The only error that may occure in an 1xxx error.                      ***/
/*****************************************************************************/
smi_error_t _smi_get_no_processes(void);

/*****************************************************************************/
/*** Determines the rank of the calling processes within the               ***/
/*** MPI-communicator MPI_COMM_WORLD and stores it in the global variable  ***/
/*** 'mpi_rank'.                                                           ***/
/*** The only error that may occure in an 1xxx error.                      ***/
/*****************************************************************************/
smi_error_t _smi_get_loc_mpi_rank(void);

/*****************************************************************************/
/*** Determines the size of a single page, as handled by the MMU. If the   ***/
/*** used machines possess different page sizes, the smallest common       ***/
/*** multiple is used. This function requires, that 'no_processes' and     ***/
/*** 'my__smi_local_rank' are already set.                                      ***/
/*** Only 1xxx and 2xxx errors can occure.                                 ***/
/*****************************************************************************/
smi_error_t _smi_get_page_size(void);

/********************************************************************************/
/*** Determines the process rank of the calling process and the machine       ***/
/*** ranks of all processes. Processes, which are executed on the same        ***/
/*** machine shall have consecutive ranks. This function requires, that       ***/
/*** 'no_processes' is already set.                                           ***/
/*** Furthermore, the global variable 'proc0_mname' is set.	       	      ***/
/*** Only 1xxx and 2xxx errors can occure.                                    ***/
/********************************************************************************/
smi_error_t _smi_set_ranks(void);

/********************************************************************************/
/*** Determine the total number of machines and store this in the global      ***/
/*** variable 'no_machines '.                                                 ***/
/*** This function cannot generate any error but return each time SMI_SUCCESS.***/
/********************************************************************************/
smi_error_t _smi_set_no_machines(void);

/********************************************************************************/
/*** This function determines whether all processes reside on the same        ***/
/*** machine or on a cluster of distinct machines. The global variable        ***/
/*** 'all_on_one' is set appropriate. This function requires that machine     ***/
/*** ranks and the number of machines are already set.                        ***/
/*** SMI_SUCCESS is returned always.                                          ***/
/********************************************************************************/
smi_error_t _smi_determine_closeness(void);

#ifdef __cplusplus
}
#endif


#endif



