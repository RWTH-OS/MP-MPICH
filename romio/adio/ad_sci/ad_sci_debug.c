/* 
 * $Id$
 *
 * debug and tracing macros
 */

#include "ad_sci_debug.h"

int _adsci_REC_DEPTH = 0;
int _adsci_DO_DEBUG  = 1;
char _adsci_REC_CHAR = '-';

/* indent output relative to depth of call stack */
static void _adsci_DMARK()
{     
  static int i;
  for (i=0; i< _adsci_REC_DEPTH; i++)
    fprintf(stderr,"%c",_adsci_REC_CHAR);
}

/* indent output relative to depth of call stack, but with 
   "error symbol */
static void _adsci_DEMARK() 
{ 
  static int i;
  for (i=0; i< _adsci_REC_DEPTH; i++)
    fprintf(stderr,"*");
}
