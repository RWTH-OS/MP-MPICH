/* $Id$ */

#ifndef _FORTRAN_BINDING_H_
#define _FORTRAN_BINDING_H_

#include "general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/* If the Fortran program is compile with the '-r8' option:                   */
/* --------------------------------------------------------                   */
/* All integers in the calling Fortran program are 8-byte values. The first 4 */
/* byte are the low-level part, the remaining 4 byte the high-level part.     */
/* Unfortunatelly, C demands this two parts just the other way round. But     */
/* because all quantities fit also in 4 byte, just the lower 4 bytes (the     */
/* first 4) are usen in this C function. To do so, each passed array is       */
/* assumed to be of type lint and just the componen '.l' ist used for         */
/* calculation                                                                */
/******************************************************************************/

typedef struct {int l; int h;} lint;




void smi_init_(lint*);
void smi_finalize_(lint*);
void smi_proc_rank_(lint*, lint*);
void smi_proc_size_(lint*, lint*);
void smi_node_rank_(lint*, lint*);
void smi_node_size_(lint*, lint*);
void smi_proc_to_node_(lint*, lint*, lint*);
void smi_create_shreg_(lint*, int*, lint*, int*, lint*);
void smi_free_shreg_(lint*, lint*);
void smi_adr_to_region_(int*, lint*, lint*);
void smi_switch_to_replication_(lint*, lint*, lint*, lint*, lint*, lint*);
void smi_switch_to_sharing_(lint*, lint*, lint*, lint*, lint*);
void smi_ensure_consistency_(lint*, lint*, lint*, lint*, lint*);
void smi_mutex_init_(lint*,lint*);
void smi_mutex_lock_(lint*,lint*);
void smi_mutex_unlock_(lint*,lint*);
void smi_mutex_destroy_(lint*,lint*);
void smi_init_shregmmu_(lint*, lint*);
void smi_imalloc_(lint*, lint*, int*, lint*);
void smi_cmalloc_(lint*, lint*, int*, lint*);
void smi_ifree_(int*, lint*);
void smi_cfree_(int*, lint*); 
void smi_page_size_(lint* pz, lint* error);
void smi_mutex_init_with_locality_(lint* id, lint* prank, lint* error);
void smi_get_timer_(lint* sec, lint* microsec, lint* error);
void smi_get_timespan_(lint* sec, lint* microsec, lint* error);
void smi_loop_init_(lint*, lint*, lint*, lint*, lint*);
void smi_get_iterations_(lint*, lint*, lint*, lint*, lint*);
void smi_loop_free_(lint*, lint*);
void smi_evaluate_speed_(double*, lint*); 
void smi_use_evaluated_speed_(lint*, lint*);
void smi_set_loop_param_(lint*, double*, lint* , lint*, lint*, lint*, lint*);
void smi_set_loop_help_param_(lint*, lint*, lint*);
void smi_loop_k_adaption_mode_(lint*, lint*, lint*, lint*);

#ifdef __cplusplus
}
#endif


#endif




