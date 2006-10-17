/* $Id: general.c,v 1.1 2004/03/19 22:14:25 joachim Exp $ */

#include "env/general_definitions.h"
#include "general.h"




#if 0
/********************************************************************************/
/* Integer Minimum is returned.                                                 */ 
/********************************************************************************/
int imin(int a, int b)
 {
   if (a<b) return(a);
   return(b);
 }
  
/********************************************************************************/
/* Integer Maximum is returned.                                                 */ 
/********************************************************************************/
int imax(int a, int b)
 {
   if (a<b) return(b);
   return(a);
 }
#endif

/********************************************************************************/
/* returns the minimum power of two, which is greater or equeal value           */
/* with a minimum valua of min                                                  */ 
/********************************************************************************/
int _smi_power_of_2_ge(int value, int min) 
{
    int i = min;
    while ( i < value )
	i <<= 1;
    
    return(i);
}



  
/********************************************************************************/
/* Build the _smi_intersection of the two ranges a1...a2 and b1...b2 and return the  */
/* result in c1...c2. If c1..c2 is empty, then values such that c2<c1 are       */
/* returned to indicate this. An error can never occure. It is assumed that     */
/* a1<a2 and b1<b2                                                              */
/********************************************************************************/
void _smi_intersection(int a1, int a2, int b1, int b2, int* c1, int* c2)
 {
   int tmp;

   
   /* sort such that a1<b1 */
   if (a1>b1)
    {
      /* swap a1..a2 and b1..b2 */
      tmp = a1;
      a1 = b1;
      b1 = tmp;
      tmp = a2;
      a2 = b2;
      b2 = tmp;
    }

   /* check if the _smi_intersection is empty */
   if (a2<b1)
    {
      /* it is */
      *c1 = 1;
      *c2 = 0;
      return;
    }

   /* otherwise, determine _smi_intersection and return */
   *c1 = b1;
   *c2 = imin(a2,b2);
   return;
 }



 
/*********************************************************************************/
/* Determine the rank of the calling process under the processes on this machine.*/
/* It is assumed that 'proc_id' is in a valid range, such that this function can */
/* never run into an error. The rank is returned.                                */ 
/*********************************************************************************/
int _smi_local_rank(int proc_id)
 {
   int i;
   int rank=0;
 
   for(i=0;i<proc_id;i++)
      if (_smi_machine_rank[i] == _smi_machine_rank[proc_id])
         rank++;
   
   return(rank);
 }
 

/*********************************************************************************/
/* Determines how many SMI processes execute on this machine. It is assumed that */
/* the parameter 'machine' is always in a valid range, such that this function   */
/* never can encounter an error. The desired quantity is returned.               */
/*********************************************************************************/
int _smi_no_processes_on_machine(int machine)
 {
   int i;
   int result = 0;
 
   for(i=0;i<_smi_nbr_procs;i++)
      if (_smi_machine_rank[i] == machine)
         result++;
 
   return(result);
 }




   
