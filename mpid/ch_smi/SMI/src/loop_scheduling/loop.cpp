/* $Id$ */

#include "loop.h"
#include "dyn_mem/dyn_mem.h"


extern "C" { 
#include "proc_node_numbers/first_proc_on_node.h"
//#include "synchronization/store_barrier.h"
}

#include <iostream>

//------------------------------------------------------------------------
///////////////////////////////////////////////////////////
// loop's static variables definition and initialization //
///////////////////////////////////////////////////////////
//------------------------------------------------------------------------
dynArray<loop*>	loop::loopArray;
int					loop::P,loop::M;	// number of processes/computing nodes
double				loop::procPredecessorSpeed,loop::procSpeed,loop::procTotalSpeed;
double				loop::kStart;
double				loop::kRemoteStart;
int					loop::myProcRank;
int					loop::myNodeRank;
#ifdef SHOW_DEBUG
ofstream 			loop::file;			// for the logfile 
#endif

//------------------------------------------------------------------------
//////////////////////////////
// member functions of loop //
////////////////////////////// 
//------------------------------------------------------------------------
//
// loop's constructor
//
loop::loop() //throw(err) 
:globalLow(1),globalHigh(0),k(kStart),kRemote(kRemoteStart)
,_smi_maxHelpDist(min(P-1,_smi_maxHelpDistDefault)),maxCalcDist(min(P-1,maxCalcDistDefault))
,dist(0),preChunkSize(0),partitionMode(0),speed(1),predecessorSpeed(myProcRank)
,totalSpeed(P),status(loopLocal),minChunkSizeLocal(minChunkSizeLocalDefault)
,minChunkSizeRemote(minChunkSizeRemoteDefault),maxChunkSizeLocal(maxChunkSizeLocalDefault)
,maxChunkSizeRemote(maxChunkSizeRemoteDefault),_smi_adaptionMode(_smi_adaptionModeDefault)

{
   int	error,procRank;
   int	i,procLower,procHigher,firstProcOnNode,lastProcOnNode;
   int	localProcs; 

   try
   {
      // init arrays
      remoteTaskTime=new double*[P];
      if(remoteTaskTime==NULL)
         throw(__LINE__);
      remoteHelpOrder=new int[P-1];
      if(remoteHelpOrder==NULL)
         throw(__LINE__);
      remoteHigh=new int[P-1];
      if(remoteHigh==NULL)
         throw(__LINE__);
      remote=new runTimeData*[P];
      if(remote==NULL)
         throw(__LINE__);
      mutexBoundsRemote=new int[P];
      if(mutexBoundsRemote==NULL)
         throw(__LINE__);

   	// init shared memory needed for loop
   	for(procRank=0;procRank<P;procRank++)
      {
      	error=SMI_Cmalloc(sizeof(runTimeData),procRank|INTERNAL,
                        	(void **)&remote[procRank]);
         if(error!=SMI_SUCCESS)
            throw(-1);      
      }
   	for(procRank=0;procRank<P;procRank++)
      {
         error=SMI_Mutex_init_with_locality(&mutexBoundsRemote[procRank],procRank);
         if(error!=SMI_SUCCESS)  // 
            throw(-2);      
   	}
   	for(procRank=0;procRank<P;procRank++)
      {
      	error=SMI_Cmalloc(sizeof(double),procRank|INTERNAL,
                        	(void **)&remoteTaskTime[procRank]);
         if(error!=SMI_SUCCESS)
            throw(-4);      
      }
      local=remote[myProcRank];
      mutexBounds=mutexBoundsRemote[myProcRank];

   	// generate search order for remote phase in array remoteHelpOrder
      DEBUG(this<<"remoteHelpOrder ");
      i=0;
      lastProcOnNode=_smi_last_proc_on_node(myNodeRank);
      firstProcOnNode=_smi_first_proc_on_node(myNodeRank);
      localProcs=lastProcOnNode-firstProcOnNode;
      procLower=myProcRank-1;
      procHigher=myProcRank+1;
      while(i<localProcs)
      {
      	if(procHigher<=lastProcOnNode) {
         	DEBUG(procHigher<<",");
            remoteHelpOrder[i++]=procHigher++;
         }
      	if(procLower>=firstProcOnNode) {
         	DEBUG(procLower<<",");
         	remoteHelpOrder[i++]=procLower--;
         }
      }
      while(i<P-1)
      {
      	if(procHigher<P) {
         	DEBUG(procHigher<<",");
            remoteHelpOrder[i++]=procHigher++;  
         }
      	if(procLower>=0) {
         	DEBUG(procLower<<",");
            remoteHelpOrder[i++]=procLower--;
         }
      }
      DEBUG(endl);
   }
   // exception handling
   catch(int code) 
   {
      int	i,errorStatus;
      
      switch(code)
      {
      	case -4: for(i=0;i<procRank;i++)
                 	{
                  	errorStatus=SMI_Cfree((char*)remoteTaskTime[i]);
                     assert(errorStatus==SMI_SUCCESS);
                  }
                  procRank=P;
      	case -2: for(i=0;i<procRank;i++)
                 	{
                  	errorStatus=SMI_Mutex_destroy(mutexBoundsRemote[i]);
                     assert(errorStatus==SMI_SUCCESS);
                  }
                  procRank=P;
      	case -1: for(i=0;i<procRank;i++)
                 	{
                  	errorStatus=SMI_Cfree((char*)remote[i]);
                     assert(errorStatus==SMI_SUCCESS);
                  }
                  break;
         default: 	error=SMI_ERR_NOMEM;
               	break;
      }                                 
      if(remoteTaskTime!=NULL)
         delete []remoteTaskTime;
      if(remoteHelpOrder!=NULL)
         delete []remoteHelpOrder;
      if(remoteHigh!=NULL)
         delete []remoteHigh;
      if(remote!=NULL)
         delete []remote;
      if(mutexBoundsRemote!=NULL)
         delete []mutexBoundsRemote;
         
      throw err("Loop's constructor failed",code,error);
   }
}
//------------------------------------------------------------------------
//
// loop's destructor
//
loop::~loop()
{
   int	error,procRank;
   
   for(procRank=0;procRank<P;procRank++)
   {
      error=SMI_Cfree((char*)remote[procRank]);
      assert(error==SMI_SUCCESS);

      error=SMI_Mutex_destroy(mutexBoundsRemote[procRank]);
      assert(error==SMI_SUCCESS);
	 
      error=SMI_Cfree((char*)remoteTaskTime[procRank]);
      assert(error==SMI_SUCCESS);
   }
   
   delete []remoteTaskTime;
   delete []remoteHelpOrder;
   delete []remoteHigh;
   delete []remote;
   delete []mutexBoundsRemote;
}
//------------------------------------------------------------------------
//
// This function just sets the global bounds of the loop
//
void loop::setGlobalBounds(const int gl,const int gh) throw(err)
{
   if(gl>gh)
      throw err("Lower bound > higher bound in setGlobalBounds",__LINE__,SMI_ERR_PARAM);
   globalLow=gl;
   globalHigh=gh;
   N=gh-gl+1;
}
//------------------------------------------------------------------------
//
// Sets the partition mode after checking the input, default = 0
//
void loop::setPartitionMode(const int mode)
{
   if(mode<0) 
      partitionMode=0;
   else 
      partitionMode=mode;
}
//------------------------------------------------------------------------
//
// Sets the maximum help distance. This value controls how many remote work 
// queues are checked before finishing the loop.  
//
void loop::setMaxHelpDist(const int newMaxHelpDist)
{
   if(newMaxHelpDist==SMI_HELP_ONLY_SMP) {
      _smi_maxHelpDist=_smi_last_proc_on_node(myNodeRank)-_smi_first_proc_on_node(myNodeRank);
      readHigh();
   }
   else
      if(newMaxHelpDist>=0) {
         _smi_maxHelpDist=min(P-1,newMaxHelpDist);
         readHigh();
      }
}
//------------------------------------------------------------------------
//
// Sets the maximum calculation distance. Value determines how many work queues 
// are included in the calculation of the average processed iterations. 
//
void loop::setMaxCalcDist(const int newMaxCalcDist)
{
   if(newMaxCalcDist>0) {
      maxCalcDist=min(P-1,newMaxCalcDist);
      readHigh();
   }
}
//------------------------------------------------------------------------
//
// Sets the speed variables to the evaluated speed and executes the partition
// again. 
//
void loop::useProcSpeed()
{
   speed=procSpeed;
   totalSpeed=procTotalSpeed;
   predecessorSpeed=procPredecessorSpeed;
   partition();
} 
//------------------------------------------------------------------------
//
// Logs the processed iterations in 'done'. If a chunk of iterations is given 
// to a processor the first half of the iterations is loged in done. After 
// executing the hole chunck the other half is loged.   
//
void loop::logDone(int chunkSize)
{
   int	tmp;
   
   if(_smi_adaptionMode!=SMI_NO_ADAPT || partitionMode==SMI_PART_ADAPTED_BLOCKED) {
      tmp=preChunkSize;
      preChunkSize=chunkSize/2;
      chunkSize+=tmp-preChunkSize;         
   	local->done=local->done+chunkSize;
   }
}
//------------------------------------------------------------------------
//
// This functions adapts the chunk size control variable k. After calculating 
// the average processed iterations (calcDoneMean()) a adaption function 
// depending on the 'adaption mode' is called.
//
void loop::adaptK()
{
   if(preChunkSize>0)			// do not adapt K at the first time 
   {
      switch(_smi_adaptionMode)
      {
      	case SMI_ADAPT_EXPO:
            adaptExpo(calcDoneMean());
            break; 
         case SMI_ADAPT_LINEAR:
            adaptLinear(calcDoneMean());
            break;
         case SMI_ADAPT_OPT:
            adaptOpt(calcDoneMean());
            break;
         case SMI_NO_ADAPT:
         default:
            break;
      }
   }
}
//------------------------------------------------------------------------
//
// Calculates the average processed iterations from maxCalcDist work queues 
// and the local work queue. 
//
int loop::calcDoneMean()
{
   int procRank,i,doneMean;

   doneMean=local->done+preChunkSize;				
   local->done=doneMean;
   preChunkSize=0;
 
   DEBUG(this<<" Done "<<doneMean<<",");
      
   for(i=0;i<maxCalcDist;i++)
   {
      procRank=remoteHelpOrder[i];
      doneMean+=remote[procRank]->done;
      DEBUG(remote[procRank]->done<<",");
   }

   doneMean=(int)((double)doneMean/(double)(maxCalcDist+1)+0.5);      
   DEBUG(" mean "<<doneMean);

   return(doneMean);
}

//------------------------------------------------------------------------
//
// Exponential adaption function
//
void loop::adaptExpo(const int doneMean)
{
   if(local->done > (doneMean-N/P/P))
      k=max(1.0,k/adaptionFactor);
   if(local->done < (doneMean-N/P/P)) 
      k=k*adaptionFactor;
      
   DEBUG(" k "<<k<<endl);
}

//------------------------------------------------------------------------
//
// Linear adaption function
//
void loop::adaptLinear(const int doneMean)
{
   if(local->done > (doneMean-N/P/P))
      k=max(1.0,k-1.0);
   if(local->done < (doneMean-N/P/P))
      k=k+1;
   DEBUG(" k "<<k<<endl);
}

//------------------------------------------------------------------------
//
// Modified exponential adaption function
//
void loop::adaptOpt(const int doneMean)
{
   if(local->done > doneMean+N/P/P)
      k=max(1.5,k/2.0);

   if(local->done < doneMean-N/P/P)
      k=min(P,k*2.0);
   DEBUG("  opt k "<<k<<endl);
}

