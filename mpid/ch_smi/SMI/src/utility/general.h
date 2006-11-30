/* $Id$ */

/********************************************************************************/
/********************************************************************************/
/*** This module containes some general purpose functions that are            ***/
/*** frequently employed within different circumstances.                      ***/
/********************************************************************************/
/********************************************************************************/



#ifndef _GENERAL_H_
#define _GENERAL_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************************/
/* Integer Minimum is returned.                                                 */ 
/********************************************************************************/
#if 0
int imin(int, int);
#else
#define imin(a,b) ((a < b) ? a : b)
#endif

/********************************************************************************/
/* Integer Maximum is returned.                                                 */ 
/********************************************************************************/
#if 0
int imax(int, int);
#else
#define imax(a,b) ((a > b) ? a : b)
#endif

/********************************************************************************/
/* returns the minimum power of two, which is greater or equeal value           */
/* with a minimum valua of min                                                  */ 
/********************************************************************************/
int _smi_power_of_2_ge(int value, int min); 

/********************************************************************************/
/* Build the _smi_intersection of the two ranges a1...a2 and b1...b2 and return the  */
/* result in c1...c2. If c1..c2 is empty, then values such that c2<c1 are       */
/* returned to indicate this. An error can never occure. It is assumed that     */
/* a1<a2 and b1<b2                                                              */
/********************************************************************************/
void _smi_intersection(int, int, int, int, int*, int*);

/*********************************************************************************/
/* Determine the rank of the calling process under the processes on this machine.*/
/* It is assumed that 'proc_id' is in a valid range, such that this function can */
/* never run into an error. The rank is returned.                                */ 
/*********************************************************************************/
int _smi_local_rank(int);

/*********************************************************************************/
/* Determines how many SMI processes execute on this machine. It is assumed that */
/* the parameter 'machine' is always in a valid range, such that this function   */
/* never can encounter an error. The desired quantity is returned.               */
/*********************************************************************************/
int _smi_no_processes_on_machine(int);

#ifdef __cplusplus
}
#endif


#endif
