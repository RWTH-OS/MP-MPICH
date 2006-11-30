/* $Id$ */

#ifndef _ERROR_COUNT_H_
#define _ERROR_COUNT_H_

#include "general_definitions.h"
#include "env/smidebug.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_SISCI
extern sci_sequence_t *_smi_node_sequence;   /* one sequence towards every node */
#endif

/* 
   external prototypes 
*/

/* validates all transfers between two calls */
smi_error_t SMI_Check_transfer(  int flags );

/* validates all transfers towards a specified address */
smi_error_t SMI_Check_transfer_addr( void *address, int flags );

/* validates all transfers between two calls towards this process */
smi_error_t SMI_Check_transfer_proc( int proc, int flags );


/* 
   internal prototypes 
*/
smi_error_t _smi_init_error_counter(int nbr_nodes);
smi_error_t _smi_get_connection_state(void);
smi_error_t _smi_probe_connection_state(int proc);

/* internal variant: validates all transfers between two calls */
#ifndef NO_SISCI
smi_error_t _smi_check_transfer( sci_sequence_t seq, int flags);
#else
smi_error_t _smi_check_transfer(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
