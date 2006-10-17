/* $Id$ */

#ifdef HAVE_CONFIG_H
/* WIN32 does not have config.h - see below */
#include "smiconfig.h"
#elif defined WIN32
#include "smiconfig_win32.h"
#else
#error No configuration file found! (config.h)
#endif

#include "cpuid.h"

#ifndef NO_SISCI
#include "sisci_api.h"

#define NO_FLAGS 0
#endif /* NO_SISCI */


#ifdef NO_SISCI
unsigned int _smi_get_CPU_cachelinesize ( void )
{
  return 64;
}

#else
unsigned int _smi_get_CPU_cachelinesize ( void )
{
  sci_query_system_t system_info;
  unsigned int cpu_cachelinesize;
  sci_error_t error;

  system_info.subcommand = SCI_Q_SYSTEM_CPU_CACHE_LINE_SIZE;
  system_info.data = &cpu_cachelinesize;
  SCIQuery( SCI_Q_SYSTEM, &system_info, NO_FLAGS, &error );
  if( error == SCI_ERR_OK )
    return cpu_cachelinesize;
  else
    return 64;
}
#endif /* NO_SISCI */


