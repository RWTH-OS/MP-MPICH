/* $Id: smi_time.h,v 1.1 2004/03/19 22:14:25 joachim Exp $ */

/********************************************************************************/
/********************************************************************************/
/*** This module containes function for timing purposes                       ***/
/********************************************************************************/
/********************************************************************************/



#ifndef _SMI_TIME_H_
#define _SMI_TIME_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* exported prototypes */

/********************************************************************************/
/* returns the current system time in seconds (doubele format)                  */
/********************************************************************************/
double SMI_Wtime(void);

/********************************************************************************/
/* for high resolution/low overhead time measurement, return only the CPU ticks */
/* "ticks" must be a 64 bit integer                                             */
/********************************************************************************/
void SMI_Get_ticks(void *ticks);

/********************************************************************************/
/* returns the current system time in seconds and microseconds                  */
/********************************************************************************/
smi_error_t SMI_Get_timer(int* sec, int* microsec);

/********************************************************************************/
/* returns the elapsed time span since SMI_Get_timer or SMI_Get_timespan has    */
/* been call the last time                                                      */
/********************************************************************************/
smi_error_t SMI_Get_timespan(int* sec, int* microsec);

/* internal prototypes */

/* this is a mini SMI_Wtime() replacement for internal timeout mechanisms */
int _smi_get_seconds (void);

/********************************************************************************/
/* initialization function of this module                                       */
/********************************************************************************/
void _smi_init_timer(void);

#ifdef __cplusplus
}
#endif


#endif



