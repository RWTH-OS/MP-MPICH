/* $Id: safety.c,v 1.1 2004/03/19 22:14:15 joachim Exp $ */

#include "safety.h"
 
int _smi_SECRead(volatile int *source,int valid_start, int valid_end)
{
  int RetVal;
  do {
      RetVal = *source;
  } while ((RetVal < valid_start) || (RetVal > valid_end));

  return(RetVal);
}
