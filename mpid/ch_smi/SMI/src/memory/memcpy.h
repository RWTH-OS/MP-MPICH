/* $Id$ */

#ifndef _SMI_MEMCPY_H_
#define _SMI_MEMCPY_H_

#include "memcpy_base.h"

#define DMA_SIZE_ALIGNMENT 8

/* initiate synchronous memory transfer 

   return values: 
   SMI_SUCCESS      = memory has been copied successfully
   SMI_ERR_PARAM    = illegal flags were supplied
   SMI_ERR_TRANSFER = sequence error during transfer - transfer potentially corrupted
*/
smi_error_t SMI_Memcpy (void *dest, void *src, size_t size, int flags);

/* initiate asynchronous transfer via DMA

   return values:
   SMI_SUCCESS      = memory transfer was initiated successfully 
   SMI_ERR_PARAM    = illegal flags were supplied
*/
smi_error_t SMI_Imemcpy (void *dest, void *src, size_t size, int flags, smi_memcpy_handle* h);

/* wait for the completion of the given transfer 

   return values:
   SMI_SUCCESS      = memory transfer has completed successfully 
   SMI_ERR_PARAM    = illegal handle was supplied
   SMI_ERR_TRANSFER = sequence error during transfer - transfer potentially corrupted
*/
smi_error_t SMI_Memwait (smi_memcpy_handle h);

/* test for the completion of the given transfer 

   return values:
   SMI_SUCCESS      = memory transfer has completed successfully 
   SMI_ERR_PENDING  = memory transfer has not yet completed
   SMI_ERR_PARAM    = illegal handle was supplied
   SMI_ERR_TRANSFER = sequence error during transfer - transfer potentially corrupted
*/
smi_error_t SMI_Memtest (smi_memcpy_handle h);

/* these are the equivalent functions to Memwait() and Memtest(), but for 
   a number of pending transfers. You have to supply the number of handles
   and the handles themselves (in an array). The individual return values 
   for each transfer are written into the status array */

/* return vaulues:
   SMI_SUCCESS      = all memory transfers have completed successfully 
   SMI_ERR_OTHER    = look at the individual status variables to find the exeact reason */
smi_error_t SMI_MemwaitAll (int count, smi_memcpy_handle *h, smi_error_t *status); 

/* return vaulues:
   SMI_SUCCESS      = all memory transfers have completed successfully 
   SMI_ERR_PENDING  = one or more memory transfers have not yet completed
   SMI_ERR_OTHER    = look at the individual status variables to find the exeact reason */
smi_error_t SMI_MemtestAll (int count, smi_memcpy_handle *h, smi_error_t *status);

#endif
