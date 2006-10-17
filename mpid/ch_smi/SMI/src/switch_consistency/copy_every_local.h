/* $Id: copy_every_local.h,v 1.1 2004/03/19 22:14:21 joachim Exp $ */

#ifndef __COPY_EVERY_LOCAL_H
#define __COPY_EVERY_LOCAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*** This function take a globally distributed and shared region and it's  ***/
/*** corresponding counterpart region. It fills the contents off all       ***/
/*** segments of the distributed shared segment, that are owned by the     ***/
/*** executing process, with the corresponding  data of the local          ***/
/*** counterpart region.                                                   ***/
/*****************************************************************************/
smi_error_t _smi_copy_every_local(int global_region_id, int local_region_id);

#ifdef __cplusplus
}
#endif


#endif
