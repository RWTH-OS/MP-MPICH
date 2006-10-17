/* $Id$ */

#ifndef _SMI_LOWLEVELMP_H__
#define _SMI_LOWLEVELMP_H__

#include <stdio.h>
#include "env/general_definitions.h"
#include "startup/startup.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/* Use this define to enable use of hierarchical intra- and inter-node barrier */
#define TWO_STEP_BARRIER 1

int _smi_ll_barrier(void);
int _smi_ll_bcast(int *buffer, size_t count, int sender_rank, int my_rank);
int _smi_ll_allgather(int *SendBuf, size_t count, int *RecvBuf, int my_rank);
boolean _smi_ll_all_true(boolean bTest);
boolean _smi_ll_all_equal_pointer(void *Pointer);
int _smi_ll_commrank(int *rank);
int _smi_ll_commsize(int *size);

void _smi_ll_init(int NodeRank, int NbrNodes, int volatile* BaseAddr,
		   int LocalRank, int NbrLocalProcs, 
		   int volatile* LocBaseAddr, 
		   int GlobalRank, int NbrProcs);

#ifdef __cplusplus
}
#endif

#endif /* _SMI_LOWLEVELMP_H__ */

