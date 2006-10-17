/* $Id$ */

#ifndef _COPY_H_
#define _COPY_H_


#ifdef __cplusplus
extern "C" {
#endif
 

smi_error_t _smi_copy_from_to(void*, void*, size_t, int);
smi_error_t _smi_copy_from_to_double(void*, void*, size_t, int);

#ifdef __cplusplus
}
#endif


#endif
