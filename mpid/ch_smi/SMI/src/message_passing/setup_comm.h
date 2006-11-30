/* $Id$ */

/*****************************************************************************/
/*****************************************************************************/
/*** This module builds two communicators: one, which is just a reordered  ***/
/*** version of MPI_COMM_WORLD, named SMI_COMM_WORLD. This one             ***/
/*** contailns all processes in the order of ther SMI ranks. Additionally, ***/
/*** for each distinct machine, a communicator is build: MPI_COMM_MACHINE, ***/
/*** which contains all processes, beeing executed on this machine.        ***/
/*****************************************************************************/
/*****************************************************************************/


#ifndef __SETUP_COMM_H
#define __SETUP_COMM_H


#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************************/
/*** Generate the communicator 'SMI_COMM_WORLD', which includes all           ***/
/*** processes, but with this reordering.                                     ***/
/*** The only errors that can occure are 1xxx and 2xxx errors.                ***/
/********************************************************************************/
smi_error_t _smi_build_reordered_communicator(void);


/********************************************************************************/
/*** Generate the communicators'MPI_COMM_MACHINE'. This communicator includes ***/
/*** all processes, which are executed on the machine of the calling process. ***/
/*** Attention: This function is a global synchronization point. Therefore,   ***/
/*** it requires, that all processes call it collectively.                    ***/
/*** The only errors that can occure are 1xxx and 2xxx errors.                ***/
/********************************************************************************/
smi_error_t _smi_build_machine_communicators(void);

#ifdef __cplusplus
}
#endif


#endif



