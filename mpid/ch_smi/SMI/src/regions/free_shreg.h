/* $Id$ */

#ifndef _FREE_SHREG_H_
#define _FREE_SHREG_H_

#ifdef __cplusplus
extern "C" {
#endif

smi_error_t SMI_Free_shreg(int);
int _smi_free_shreg(int);
int _smi_free_busy_shregs(void);
void _smi_free_remaining_region_structs(void);

#ifdef __cplusplus
}
#endif


#endif
