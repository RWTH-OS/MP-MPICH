/* $Id: loop_get.cpp,v 1.1 2004/03/19 22:14:16 joachim Exp $ */

#include "loop.h"
//------------------------------------------------------------------------
//////////////////////////////
// member functions of loop //
////////////////////////////// 
//------------------------------------------------------------------------
// loop's getLocal:
// A chunk of n/k iterations is taken from the local work queue (local bounds,
// n = remaining iterations). The lower and higher bound of this chunk is 
// returned in "low" and "high". Unless the iterations of the chunk are the
// last of the local work queue, the chunk size is equal or higher 
// "minChunkSize".  
//------------------------------------------------------------------------
// getLocal for TIMED SMI_SHM_BLOCKED
void loop::getLocalTimed(int& low,int& high)
{
   low=local->lo;
   high=local->hi;
   local->lo=high+1;
   status=loopReady;
   DEBUG(this<<" local "<<low<<"-"<<high<<endl); 
   taskTime.start();
}
//------------------------------------------------------------------------
// getLocal for CYCLIC 
void loop::getLocalCyclic(int& low,int& high)
{
   int	l=1,h=0,chunkSize=0,n= -1;

   lockBounds();
      chunkSize=local->hi - local->lo +1;
      if(chunkSize>0) {
         n=chunkSize;
         if(chunkSize>1) {
            chunkSize/=P;
            if(chunkSize>minChunkSizeLocal) {
               chunkSize=(int)((double)chunkSize/k+0.5); // always asumed: k >=1 
               if(chunkSize<minChunkSizeLocal)
                  chunkSize=minChunkSizeLocal;
            }
			if(chunkSize>maxChunkSizeLocal)
				chunkSize=maxChunkSizeLocal;
         }
     	   l=local->lo;   
         h=l+(chunkSize-1)*P;
         local->lo=h+P;
      }
   unlockBounds();

   if(chunkSize>0)  {
      if(chunkSize==n)
         status=loopRemote; 
      logDone(chunkSize);
   }
   else
      status=loopRemote; 

   low=l;
   high=h;
   DEBUG(this<<" local "<<l<<"-"<<h<<endl); 
}
//------------------------------------------------------------------------
// getLocal for SMI_SHM_BLOCKED
void loop::getLocalBlocked(int& low,int& high)
{
   int	l=1,h=0,chunkSize=0,n= -1;

   lockBounds();
      chunkSize=local->hi - local->lo +1;
      if(chunkSize>0) {
         n=chunkSize;
         if(chunkSize>minChunkSizeLocal) {
            chunkSize=(int)((double)chunkSize/k+0.5); // always asumed: k >=1 
            if(chunkSize<minChunkSizeLocal)
               chunkSize=minChunkSizeLocal;
         }
		 if(chunkSize>maxChunkSizeLocal)
			chunkSize=maxChunkSizeLocal;
     	 l=local->lo;   
         h=l+chunkSize-1;      
         local->lo=h+1;
      }
   unlockBounds();


   if(chunkSize>0) {
      if(chunkSize==n)
         status=loopRemote; 
      logDone(chunkSize);
   }
   else
      status=loopRemote; 

   low=l;
   high=h;
   DEBUG(this<<" local "<<l<<"-"<<h<<endl); 
}
//------------------------------------------------------------------------
// loop's getRemote:
// If the local work queue is empty, chunks of iterations are assigned from
// remote work queues. This is done in the same manner as in getLocal. The remote 
// processes considered are those stored in the remoteHelpOrder array.  
//------------------------------------------------------------------------
// getRemote for CYCLIC
void loop::getRemoteCyclic(int& low,int& high)
{
   int		procRank;
   int		l=1,h=0,chunkSize=0;
   int 		tmpLow;     
   
   while(dist<_smi_maxHelpDist)
   {
      procRank=helpFirstProc();
      if(remoteHigh[dist]<remote[procRank]->lo) {
         dist++;
         continue;
      }
   
      lockBoundsRemote(procRank);
         tmpLow=remote[procRank]->lo;
         chunkSize=remoteHigh[dist] - tmpLow  +1;
         if(chunkSize>0) {
            if(chunkSize>1) {
               chunkSize/=P;
               if(chunkSize>minChunkSizeRemote) {
                  chunkSize=(int)((double)chunkSize/kRemote+0.5); // always asumed: kRemote >=1 
                  if(chunkSize<minChunkSizeRemote)
                     chunkSize=minChunkSizeRemote;
               }
			   if(chunkSize>maxChunkSizeRemote)
					chunkSize=maxChunkSizeRemote;
            }
            l=tmpLow;   
            h=l+(chunkSize-1)*P;
            remote[procRank]->lo=h+P;
         }
      unlockBoundsRemote(procRank);
                  
      if(chunkSize>0) {
         logDone(chunkSize);
         break; // out of while loop
      }
   	dist++;
   } // while loop

   if(l>h) {
      status=loopReady; 
      logDone(0);
   }

   low=l;
   high=h;
   DEBUG(this<<" remote "<<l<<"-"<<h<<" from "<<procRank<<endl); 
}
//------------------------------------------------------------------------
// getRemote for SMI_SHM_BLOCKED
void loop::getRemoteBlocked(int& low,int& high)
{
   int		procRank;
   int		l=1,h=0,chunkSize=0;
   int		tmpLow,tmpHigh;
   
   while(dist<_smi_maxHelpDist)
   {
      procRank=helpFirstProc();
      tmpHigh=remoteHigh[dist];
      if(tmpHigh<remote[procRank]->lo) {
         dist++;
         continue;
      }
      
      lockBoundsRemote(procRank);
         tmpLow=remote[procRank]->lo;
         chunkSize=tmpHigh - tmpLow +1;
         if(chunkSize>0) {
            if(chunkSize>minChunkSizeRemote) {
               chunkSize=(int)((double)chunkSize/kRemote+0.5); // always assumed: kRemote >=1 
               if(chunkSize<minChunkSizeRemote)
                  chunkSize=minChunkSizeRemote;
            }
			if(chunkSize>maxChunkSizeRemote)
				chunkSize=maxChunkSizeRemote;
            l=tmpLow;  
            h=l+chunkSize-1;      
            remote[procRank]->lo=h+1;
         }
      unlockBoundsRemote(procRank);
      
            
      if(chunkSize>0) {
         logDone(chunkSize);
         break; // out of while loop
      }
   	dist++;
   } // while loop

   if(l>h) {
      status=loopReady; 
      logDone(0);
   }

   low=l;
   high=h;
   DEBUG(this<<" remote "<<l<<"-"<<h<<" from "<<procRank<<endl); 
}
//------------------------------------------------------------------------
//
// returns process rank of the next work queue to be considered
//
int loop::helpFirstProc()
{
   return(remoteHelpOrder[dist]);
}

