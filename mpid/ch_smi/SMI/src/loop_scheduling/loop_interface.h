/* $Id: loop_interface.h,v 1.1 2004/03/19 22:14:16 joachim Exp $ */

#ifndef __LOOP_INTERFACE_H
#define __LOOP_INTERFACE_H

#include <env/general_definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------*/
smi_error_t SMI_Loop_scheduling_init(void);
/*----------------------------------------------------------------------*/
smi_error_t SMI_Loop_init(int* const id,const int globalLow,const int globalHigh
               	,int mode);
/* mode: */
#define SMI_PART_BLOCKED 1
#define SMI_PART_CYCLIC  2
#define SMI_PART_ADAPTED_BLOCKED 3
#define SMI_PART_TIMED_BLOCKED 4
/*----------------------------------------------------------------------*/
smi_error_t SMI_Loop_free(const int id);
/*----------------------------------------------------------------------*/
smi_error_t SMI_Get_iterations(const int id,int* const status,int* const low,
                     	int* const high);
/* status: */
#define SMI_LOOP_READY 	0
#define SMI_LOOP_LOCAL	1
#define SMI_LOOP_REMOTE 2


/*----------------------------------------------------------------------*/
/*////////////////////*/
/* Advanced functions */
/*////////////////////*/
/*----------------------------------------------------------------------*/
/* use SMI_NO_CHANGE for no change of a particular parameter */
#define SMI_NO_CHANGE 0
/*----------------------------------------------------------------------*/
smi_error_t SMI_Evaluate_speed(double* const speedArray); 
/*----------------------------------------------------------------------*/
smi_error_t SMI_Use_evaluated_speed(const int id);
/*----------------------------------------------------------------------*/
/* use SMI_NO_CHANGE for no change of a particular parameter */
smi_error_t SMI_Loop_k_adaption_mode(const int id,const int _smi_adaptionMode, 
                        	const int maxCalcDist);
/* _smi_adaptionMode: */
#define SMI_NO_ADAPT 1
#define SMI_ADAPT_EXPO 2
#define SMI_ADAPT_LINEAR 3
#define SMI_ADAPT_OPT 4
/*----------------------------------------------------------------------*/
/* use SMI_NO_CHANGE for no change of a particular parameter */
smi_error_t SMI_Set_loop_param(const int id,const double kNew,
                  	const int minChunkSizeLocal,const int minChunkSizeRemote,
					const int maxChunkSizeLocal,const int maxChunkSizeRemote);
/*----------------------------------------------------------------------*/
smi_error_t SMI_Set_loop_help_param(const int id, const int _smi_maxHelpDist);
/* set _smi_maxHelpDist to SMI_HELP_ONLY_SMP to allow finished processes only to help
other processes if there are one the same node  (locality) */ 
#define SMI_HELP_ONLY_SMP -1

#ifdef __cplusplus
}
#endif

#endif
