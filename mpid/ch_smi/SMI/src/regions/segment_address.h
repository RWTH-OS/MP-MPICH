/* $Id$ */

#ifndef _SEGMENT_ADDRESS_H_
#define _SEGMENT_ADDRESS_H_
 
#include "env/general_definitions.h"
 
#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32

#define VIRTUAL_MEM_MAX 0x7f000000

#define VIRTUAL_MEM_MIN 0x00010000
#endif


#ifdef WIN32
extern char *address_counter;
#endif

/*****************************************************************************/
/*** Internal function: determine a free portion of the virtual address    ***/
/*** space that is 'size' byte large.                                      ***/
/*** Error-Codes: System-Errors											   ***/
/***              MPI-Errors											   ***/
/***              SMI_ERR_OTHER (not enough virtual memory)				   ***/
/*****************************************************************************/
smi_error_t _smi_determine_start_address(char** address, size_t size);

#ifdef __cplusplus
}
#endif


#endif
