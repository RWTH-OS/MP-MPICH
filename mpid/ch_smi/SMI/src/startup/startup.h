/* $Id: startup.h,v 1.1 2004/03/19 22:14:21 joachim Exp $ */

#ifndef _SMI_STARTUP_H__
#define _SMI_STARTUP_H__

#include <stdio.h>
#include "env/general_definitions.h"
#include "sciflush.h"
#include "message_passing/lowlevelmp.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define SMP_STARTUP_TIMEOUT 5
#define SYNC_STATIC_TIMEOUT 20
#define SYNC_PERPROC_TIMEOUT 5

#define BASESEGMENT_DYNAMIC_SIZE 1

/* dynamic size of the SMI basesgmt */
#define BS_MIN    (64*1024)
#define BS_INC    (4*1024)
#if BASESEGMENT_DYNAMIC_SIZE
#define BASESGMT_SIZE(n) (BS_MIN +  BS_INC * n)
#else
#define BASESGMT_SIZE(n) (2*BS_MIN)
#endif
     
typedef struct smi_sci_info_t_ {
  int iNbrLocalAdapter;
  int iDefaultAdapter;
  int SciIds[MAX_ADAPTER];
} smi_sci_info_t;

/* offsets for internal services in the base segment */
#define BASESGMT_OFFSET_FLUSH 0
#define BASESGMT_OFFSET_INIT       (BASESGMT_OFFSET_FLUSH + (FLUSH_SIZE/sizeof(int)))
#define BASESGMT_OFFSET_BARRIER    (BASESGMT_OFFSET_INIT + (INTS_PER_STREAM*nbr_procs))
#define BASESGMT_OFFSET_ALIVECHECK (BASESGMT_OFFSET_BARRIER + (2*INTS_PER_STREAM*nbr_procs))
#define BASESGMT_OFFSET_DATASHARE  (BASESGMT_OFFSET_ALIVECHECK + (INTS_PER_STREAM*nbr_procs))

/* generic functions which call the specific functions below */
smi_error_t _smi_lowlevel_init (smi_args_t *sArgs);
smi_error_t _smi_lowlevel_shutdown (void);

#ifndef NO_SISCI
/* SCI specific */
smi_error_t _smi_sci_startup(smi_args_t *sArgs);
smi_error_t _smi_sci_shutdown(void);
smi_error_t _smi_QuerySCI(sci_desc_t SCIfd, int sync_id);
smi_error_t _smi_OpenSCI(sci_desc_t* pSCIfd, int* pSCIid, smi_args_t *sArgs);
#endif /* NO_SISCI */

/* SMP specific */
smi_error_t _smi_smp_startup(smi_args_t *sArgs);
smi_error_t _smi_smp_shutdown(void);

#ifdef __cplusplus
}
#endif


#endif /* _SMI_STARTUP_H__ */
