/* $Id$ */

#ifndef _REGION_LAYOUT_H_
#define _REGION_LAYOUT_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************/
/*** A structure that describes a region                                   ***/
/*****************************************************************************/

#if 0 /* already defined in smi.h */
typedef struct
 {
   int nsegments;    /* number of comprising segments             */
   size_t size;      /* total region size                         */
   char* adr;        /* region start address                      */
   int* seg_size;    /* size of each segment                      */
   char** seg_adr;   /* start address of each segment             */
   int* seg_machine; /* physical machine location of each segment */
 } smi_rlayout_t;
#endif  /* already defined in smi.h */




/**********************************************************************/
/* This functions allocates a structure of type 'rlayout_t' and       */
/* returnes in it the layout of a shared memory region. I.e. the      */
/* number of segments of that it is composed, their sizes (in Byte),  */
/* their start address and their physical machine location. In case   */
/* that SMI has not been initialized before, this function returnes   */
/* SMI_ERR_NOINIT. In the case that a region with the specified ID    */
/* does not exists or is an internal one, SMI_ERR_PARAM is returned.  */
/**********************************************************************/
smi_error_t SMI_Region_layout(int region_id, smi_rlayout_t** r);

/* internal */
region_t *_smi_get_region (int shreg_id);
region_t *_smi_address_to_region(char* address);
int _smi_address_to_region_id(char* address);
int _smi_address_to_node(char* address);
size_t _smi_get_region_size (int region_id);

#ifdef __cplusplus
}
#endif


#endif
