/* $Id: svm_shmem.h,v 1.1 2004/03/19 22:14:17 joachim Exp $ */

/*****************************************************************************/
/*****************************************************************************/
/***                                                                       ***/
/*** This module supplies all basic functions to deal with SVM shared      ***/
/*** segments.                                                             ***/
/***                                                                       ***/
/*****************************************************************************/
/*****************************************************************************/

#ifndef __SVM_SHMEM_H
#define __SVM_SHMEM_H

#ifdef __cplusplus
extern "C" {
#endif


#include "env/general_definitions.h"



/*****************************************************************************/
/*** Initialization of some internal data structures.                      ***/
/*****************************************************************************/
smi_error_t _smi_init_svm_subsystem();
 



/*****************************************************************************/
/*** This functions maps a SVM shared segment with the specified id in the ***/
/*** calling processes address space.                                      ***/
/*****************************************************************************/
smi_error_t _smi_map_svm_shared_segment(shseg_t* shseg);
 

  
/*****************************************************************************/
/*** This functions releases the mapping of the specified segment.         ***/
/*** Afterwards it is no longer mapped into the address space of the       ***/
/*** calling process.                                                      ***/
/*****************************************************************************/
smi_error_t _smi_unmap_svm_shared_segment(shseg_t* shseg);

  

/*****************************************************************************/
/*** Creates a SVM shared segments of the specified size on the specified  ***/
/*** machine. The resulting identifier is passed back. Therefore, this     ***/
/*** function results in a global synchronization.                         ***/
/*****************************************************************************/
smi_error_t _smi_create_svm_shared_segment(shseg_t* shseg);

  
  
/*****************************************************************************/
/*** Removes a SVM shared segment with the specified identifier.           ***/
/*****************************************************************************/
smi_error_t _smi_remove_svm_shared_segment(shseg_t* shseg);


#ifdef __cplusplus
}
#endif
  

#endif



