/* $Id$ */

#ifndef _SMI_QUERY_H_
#define _SMI_QUERY_H_

#if defined SOLARIS_X86 || defined SOLARIS_SPARC
#include <sys/processor.h>
#include <sys/procset.h>
#endif

#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "env/error_count.h"

/* exported */

/* SMI_Query is used to retrieve various parameters from the 
   shared memory system which might be required by the application
   to optimize performance or configure to certain environments */

smi_error_t SMI_Query (smi_query_t cmd, int arg, void *result);

void _smi_set_adapter_available(int adapter_nbr);
void _smi_set_adapter_unavailable(int adapter_nbr);
int _smi_adapter_available(int adapter_nbr);

extern int _smi_sci_errcnt;

/* internal */
smi_error_t _smi_init_query (int nbr_procs);
void _smi_finalize_query (void);
smi_error_t _smi_topology_init(void);

#endif
