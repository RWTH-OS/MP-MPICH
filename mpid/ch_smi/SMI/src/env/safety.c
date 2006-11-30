/* $Id$ */

#include "safety.h"
 
int _smi_SECRead(volatile int *source,int valid_start, int valid_end)
{
  int RetVal;
  do {
      RetVal = *source;
  } while ((RetVal < valid_start) || (RetVal > valid_end));

  return(RetVal);
}
