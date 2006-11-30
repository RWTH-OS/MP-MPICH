/* $Id$ */

#ifndef _SMI_SIGNALIZATION_H_
#define _SMI_SIGNALIZATION_H_

#include <pthread.h>

#include "env/general_definitions.h"
#include "env/smidebug.h"


#define SMI_SIGNAL_TRY_NUM 100
#define SMI_SIGNAL_TRY_DELAY (10*1000)

/* SMI_Signal_wait  blockiert solange, bis dem angegebenen Prozess ein Interrupt 
   kommt. Wenn als proc_rank SMI_SIGNAL_ANY (zu -1 definiert) angegeben ist, kann 
   der Interrupt von einem beliebigen Prozess kommen */
smi_error_t SMI_Signal_wait (int proc_rank);
/* SMI_Signal_setCallBack() erzeugt einen Thread, der auf den Interrupt wartet 
   und anschliessend die angegebene Funktion aufruft. 
   In dieem Fall wird ein gueltiger signal_handle zurueckgegeben, der fuer
   SMI_Signal_joinCallBack verwendet werden kann.*/  
smi_error_t SMI_Signal_setCallBack (int proc_rank, void (*callback_fcn)(void *), 
				void *callback_arg, smi_signal_handle* h);
/* SMI_Signal_joinCallBack wartet solange, bis dass die callback-Funktion zu dem
   angegebenen handle abgearbeitet worden ist. */
smi_error_t SMI_Signal_joinCallBack (smi_signal_handle* h);

/* SMI_Signal_send sendet ein Signal an den angegebenen Prozess. Wenn man
   als proc_rank SMI_SIGNAL_BCAST angibt (zu -1 definiert), wird ein 
   Signal an alle Prozesse geschickt */
smi_error_t SMI_Signal_send ( int proc_rank );


int _smi_signal_init(int, int);
int _smi_signal_finalize(void);

#endif /* _SMI_SIGNALIZATION_H_ */
