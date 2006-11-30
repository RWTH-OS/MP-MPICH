/* $Id$ */

#include "loop.h"
#include "synchronization/barrier.h"

//------------------------------------------------------------------------
//////////////////
// test globals //
//////////////////
int	count=0;

//------------------------------------------------------------------------
//////////////////////////////
// member functions of loop //
//////////////////////////////
//------------------------------------------------------------------------
//
// Distributes the iterations among the work queues as BLOCKS. Block-size 
// depends on the speed variables. 
//
void loop::blockedPartition()
{
   const double	blockSize=(double)N/totalSpeed;	// blockSize for speed=1
   
   local->lo=globalLow+(int)(blockSize*predecessorSpeed+0.5);
   local->hi=globalLow+(int)(blockSize*(predecessorSpeed+speed)+0.5)-1;
}
//------------------------------------------------------------------------
//
// Distributes the iterations among the work queues CYCLIC. Each work queue 
// gets almost the same number of iterations (depending on divisibility of N/P).
// The iterations are stored as lower and higher bound like in blockedPartition.
//
void loop::cyclicPartition()
{
   int	blockSize,rest,low;
   
   if(N>myProcRank) {
      blockSize=N/P;
      rest=N%P;
      low=globalLow+myProcRank; 
      local->lo=low;
      if(myProcRank<rest)
         local->hi=low+blockSize*P;
      else
         local->hi=low+(blockSize-1)*P;
   }
   else {
      local->lo=1;
      local->hi=0;
   }
}
//------------------------------------------------------------------------
//
// Distributes the iterations among the work queues as BLOCKS. The block-size
// of each work queue is set equal to the number of iterations processed in the
// PREVIOUS run. 
//
void loop::adaptedBlockedPartition()
{
   int	procRank,low;
   error_t error;
   
   low=globalLow;
   
   for(procRank=0;procRank<myProcRank;procRank++)
      low+=remote[procRank]->done;
   local->lo=low;
   local->hi=low+local->done-1;
   
   error=SMI_Barrier();
   assert(error==SMI_SUCCESS);
}
//------------------------------------------------------------------------
//
// Distributes the iterations among the work queues as BLOCKS. Block-size 
// depends on the relative time needed for all iterations of the local work 
// queue in the previous run. This partition does not have a remote phase. 
// During one execution of the loop no load balancing is performed. 
//
void loop::timedBlockedPartition()
{
   double 	longestTime=0.0,tmp;
   int		procRank;
   double   newPredecessorSpeed,newTotalSpeed,newSpeed;
   
   DEBUG(this<<" Speed: taskTime ");

   newPredecessorSpeed=0.0;
   for(procRank=0;procRank<myProcRank;procRank++) {
      tmp= *(remoteTaskTime[procRank]);
      newPredecessorSpeed+=1.0/tmp;
      if(longestTime < tmp)
         longestTime = tmp;
      DEBUG(tmp<<",");
   }
   newTotalSpeed=newPredecessorSpeed;
   for(procRank=myProcRank;procRank<P;procRank++) {
      tmp= *(remoteTaskTime[procRank]);
      newTotalSpeed+=1.0/tmp;
      if(longestTime < tmp)
         longestTime = tmp;
   DEBUG(tmp<<",");
   }
   
   newTotalSpeed*=longestTime;
   newPredecessorSpeed*=longestTime;
   newSpeed=1.0/(*(remoteTaskTime[myProcRank])/longestTime);
   DEBUG(";new speed param. "<<newPredecessorSpeed<<","<<newSpeed<<","<<newTotalSpeed);

   predecessorSpeed=(predecessorSpeed+newPredecessorSpeed)/2;
   totalSpeed=(totalSpeed+newTotalSpeed)/2;
   speed=(speed+newSpeed)/2;
   DEBUG(endl<<";final speed param. "<<predecessorSpeed<<","<<speed<<","<<totalSpeed<<endl );
      
   blockedPartition();   
}
//------------------------------------------------------------------------
// loop's partition:
// The global loop is parted among the processes.  
//
void loop::partition()
{
   error_t error;
   
   DEBUG(this<<" 1st partition "<<count++);
   
   switch(partitionMode)
   {
      case SMI_PART_CYCLIC:
         cyclicPartition();
         break;
      case SMI_PART_ADAPTED_BLOCKED:
      case SMI_PART_TIMED_BLOCKED:
      case SMI_PART_BLOCKED:
      default:
         blockedPartition();
         break;
   }
   local->done=0;

   DEBUG(" bounds "<<local->lo<<" - "<<local->hi<<endl); 

   error=SMI_Barrier();
   assert(error==SMI_SUCCESS);

   if(partitionMode!=SMI_PART_TIMED_BLOCKED) 
      readHigh();

}
//------------------------------------------------------------------------
// loop's rePartition: 
// Does the same as initialPartition for the second and all folowing runs of
// the loop. However runtime information might be used for chosing the block
// size to assign.   
void loop::rePartition()
{
   error_t error;
   
   if(partitionMode==SMI_PART_TIMED_BLOCKED) {
      taskTime.stop();
      *(remoteTaskTime[myProcRank])=taskTime.elapsedTime();
   }
	
   error=SMI_Barrier();
   assert(error==SMI_SUCCESS);

   switch(partitionMode)
   {
      case SMI_PART_ADAPTED_BLOCKED:
         adaptedBlockedPartition();
         break;
      case SMI_PART_TIMED_BLOCKED:
         timedBlockedPartition();
         break;	 
      case SMI_PART_CYCLIC:
         cyclicPartition();
         break;
      case SMI_PART_BLOCKED:
      default:
         blockedPartition();
         break;
   }
   local->done=0;

   DEBUG(this<<" partition "<<count++<<" bounds "<<local->lo<<" - "<<local->hi<<endl); 

   error=SMI_Barrier();
   assert(error==SMI_SUCCESS);

   dist=0;
   k=kStart;
   kRemote=kRemoteStart;
   status=loopLocal;
   preChunkSize=0;

   if(partitionMode!=SMI_PART_TIMED_BLOCKED) 
      readHigh();
}
//------------------------------------------------------------------------
//
// Reads the higher bounds from the remote work queues and stores them in the
// local array remoteHigh. This reduces remote memory accesses..
//
void loop::readHigh()
{
   int i;
   
   DEBUG(this<<" remoteHigh ");
   for(i=0;i< max(_smi_maxHelpDist,maxCalcDist);i++) {
      remoteHigh[i]=remote[remoteHelpOrder[i]]->hi;
      DEBUG(remoteHigh[i]<<"; ");
   }
   DEBUG(endl);
}
