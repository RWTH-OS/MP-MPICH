/* $Id$ */

#ifndef _SMI_SCIFLUSH_H__
#define _SMI_SCIFLUSH_H__

#include <stdio.h>
#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define FLUSH_SIZE ALLSTREAMS_SIZE

int _smi_flush_write_buffers(void); 
int _smi_flush_read_buffers(void); 

#ifndef NO_SISCI
void _smi_init_flush(int volatile *BaseAddr, sci_map_t mpisgmt_map);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SMI_SCIFLUSH_H__ */


