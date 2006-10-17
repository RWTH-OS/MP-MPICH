/* $Id: loop_static.cpp,v 1.1 2004/03/19 22:14:16 joachim Exp $ */

#include "loop.h"
#include "proc_node_numbers/proc_rank.h"
#include "proc_node_numbers/proc_size.h"
#include "proc_node_numbers/node_rank.h"
#include "proc_node_numbers/node_size.h"
#include "synchronization/barrier.h"
#include "dyn_mem/dyn_mem.h"



//------------------------------------------------------------------------
//
// inits the static variables and opens the logfile
//
void loop::initStatic() throw(err) 
{
   int	error;
   
   error=SMI_Proc_rank(&myProcRank);
   if(error!=SMI_SUCCESS)
      throw err("Loop's initStatic failed in SMI function",__LINE__,error);
   error=SMI_Node_rank(&myNodeRank);
   if(error!=SMI_SUCCESS)
      throw err("Loop's initStatic failed in SMI function",__LINE__,error);
   error=SMI_Proc_size(&P);
   if(error!=SMI_SUCCESS)
      throw err("Loop's initStatic failed in SMI function",__LINE__,error);
   error=SMI_Node_size(&M);
   if(error!=SMI_SUCCESS)
      throw err("Loop's initStatic failed in SMI function",__LINE__,error);

   kStart=kDefault;
   kRemoteStart=kRemoteDefault;
   procPredecessorSpeed=myProcRank;
   procSpeed=1;
   procTotalSpeed=P;
      
#ifdef SHOW_DEBUG            
   _smi_openDebugFile(&file,myProcRank);
#endif

}
//------------------------------------------------------------------------
//
//	Tests the speed of all processors. The speed of the slowest processor is 
// set to one and the others are assigned a speed value relative to the slowest.  	
// The speed values are stored in the proc...Speed variables and are returned 
// in the speedArray (to be used by the programmer)
//
void loop::evaluateSpeed(double* const speedArray) throw(err)
{
   double	*remoteSpeed;
   timer		kernelTime;							 
   int		procRank,error;
   double	longest;
      
   try
   {
      error=SMI_Cmalloc(P*sizeof(double),0|INTERNAL,(void **)&remoteSpeed);		 
      if(error!=SMI_SUCCESS)
         throw	(-1);
      error=SMI_Barrier();
      if(error!=SMI_SUCCESS)
         throw (__LINE__);

      kernelTime.start();
   	
      // speed test application
      // start

   	double			A,B;
      int				i,j,a,b;
   
      a=b=3;
      B=12.4343;
      A=0.0;
   
      
   	for(j=0;j<100;j++)
      {
      	for(i=0;i<5000;i++)
         {
         	A=A*i*0.1+B*12.4+(double)a-(double)b;
            B=(B*1.1)/(13.2*i+1.0);
            a=(int)B+b/(a+1);
            b=((b+1)%20)*j;
         }
      }
      // speed test application
      // end
      kernelTime.stop();
      
      remoteSpeed[myProcRank]=kernelTime.elapsedTime();
      error=SMI_Barrier();
      if(error!=SMI_SUCCESS)
         throw (__LINE__);
      
      longest=0.0;
      for(procRank=0;procRank<P;procRank++) 
      {
         speedArray[procRank]=remoteSpeed[procRank];
         if(speedArray[procRank]>longest)
            longest=speedArray[procRank];
      }

      error=SMI_Cfree((char*)remoteSpeed);
      if(error!=SMI_SUCCESS)
         throw (__LINE__);

   	DEBUG("Speed ");
      procPredecessorSpeed=0.0;
      for(procRank=0;procRank<myProcRank;procRank++)
      {
      	speedArray[procRank]=1/(speedArray[procRank]/longest);
         procPredecessorSpeed+=speedArray[procRank];
         DEBUG(speedArray[procRank] << " , ");
      }
      procTotalSpeed=procPredecessorSpeed;
      for(procRank=myProcRank;procRank<P;procRank++)
      {
      	speedArray[procRank]=1/(speedArray[procRank]/longest);
         procTotalSpeed+=speedArray[procRank];
         DEBUG(speedArray[procRank] << " , ");
      }
      DEBUG(endl);
      
      procSpeed=speedArray[myProcRank];
   
   } // try
   catch(int line)
   {
      if(line >= 0)
      {
      	error=SMI_Cfree((char*)remoteSpeed);
         assert(error==SMI_SUCCESS);
      }
      throw err("Loop's evaluateSpeed failed in SMI functions",line,error);
   }
}

//------------------------------------------------------------------------
//
// Adds a new loop to the loopArray and returns its id. 
//
int loop::addLoop() throw(err)
{
   int id; 
   
   id=++loopArray;
   loopArray[id]=new loop;
   if(loopArray[id]==NULL)
      throw err("Memory exhausted in loop's addLoop",__LINE__,SMI_ERR_NOMEM);
   return(id);
}
//------------------------------------------------------------------------
//
// Returns the corresponding pointer to an id of a loop. 
//
loop* loop::giveLoop(const int id) throw(err) 
{
  loop* temp;
  
  temp=loopArray[id];
  if(temp==NULL)
     throw err("Loop is not in loopArray in loop's giveLoop",__LINE__,SMI_ERR_PARAM); 
  return temp;
} 

