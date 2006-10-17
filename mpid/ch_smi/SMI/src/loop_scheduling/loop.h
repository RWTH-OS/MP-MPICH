/* $Id: loop.h,v 1.1 2004/03/19 22:14:16 joachim Exp $ */

#ifndef __LOOP_H
#define __LOOP_H

#include <assert.h>
#include <sys/time.h>
#include <math.h>


#include "err.h"
#include "dynarray.h"
#include "timer.h"
#include "loop_interface.h"
#include "synchronization/mutex.h"
extern "C" {
#include "env/general_definitions.h"	
// INTERNAL
// boolean 
// boolean _smi_initialized
// boolean debug
// SMI_ERR....
}

#ifdef SHOW_DEBUG
   #include <fstream.h>
#endif

//------------------------------------------------------------------------
///////////////
// constants //
///////////////
// default values of the parameters
const int minChunkSizeLocalDefault =1;
const int minChunkSizeRemoteDefault=1;
const int maxChunkSizeLocalDefault =999999999;
const int maxChunkSizeRemoteDefault=999999999;
const int _smi_adaptionModeDefault=SMI_NO_ADAPT;
const double adaptionFactor=2.0;

const int _smi_maxHelpDistDefault=16;
const int maxCalcDistDefault=16;


#define kDefault P
#define kRemoteDefault P
#define helpModeDefault helpFirst
//------------------------------------------------------------------------
///////////////
// functions //
/////////////// 
//------------------------------------------------------------------------
//
// universal min/max functions
//
#ifndef WIN32
#define min(a, b) ((a>b)?(b):(a))
#define max(a, b) ((a>b)?(a):(b))
#endif
//------------------------------------------------------------------------
////////////////
// loop class //
////////////////
//------------------------------------------------------------------------
enum loopStatus {loopLocal, loopRemote, loopReady};

class loop 
{
private:
   static dynArray<loop*>	loopArray;	// array the manage instances of loop
   static int		P,M;	// number of processes/computing nodes
   static double	procPredecessorSpeed,procSpeed,procTotalSpeed;	// processor speed variables
   static double	kStart;	// start value for k (local)
   static double	kRemoteStart; // start value for k (remote)
   static int		myProcRank; // process rank of this process
   static int		myNodeRank;	// node rank of this process
#ifdef SHOW_DEBUG
   static ofstream file;	// for the logfile
#endif

	struct runTimeData
   {
   public:
      volatile int	lo;	// lower loop boundary(local); local RW, remote RW
      volatile int	hi;	// higher loop boundary(local); local RW, remote R
      volatile int	done;	//	processed iterations; local RW, remote R	
   };
   
   int 			mutexBounds;	// mutex(for bounds(lo-hi)) id (local)
   int*			mutexBoundsRemote; // pointer to mutex id array (remote)
   runTimeData *local;		// pointer to the runTimeData (local)
   runTimeData	**remote; 	// pointer to the runTimeData array (remote)
   double**		remoteTaskTime;	// pointer to task time array (only used by timed blocked partition)
   int*			remoteHelpOrder;	// pointer to search order (remote phase) array
   int*		 	remoteHigh;	// pointer to the higher bounds array (remote)
                        	// the higher bound is only read once and then stored in this array
   
   int			globalLow,globalHigh; // global loop bounds
   int			N;	// total number of iterations (calculated from the global bounds) 
   int			_smi_maxHelpDist; // max. number of work queues considered in the remote phase
   int			maxCalcDist; // max. distance of processes considered in calcualtions
   int			dist;		// index in the remoteHelpOrder array
   double		k,kRemote;	// chunk-size control variable (local/remote)
   int 			minChunkSizeLocal,minChunkSizeRemote; // the minimum number of iterations for a chunk (local/remote)
   int 			maxChunkSizeLocal,maxChunkSizeRemote; // the maximum number of iterations for a chunk (local/remote)
   loopStatus	status;	// contains the status of the loop
   int			preChunkSize;	// buffer for the half number of iterations which are currently processed (logDone)
   int 			_smi_adaptionMode;	// the adaption mode
   int			partitionMode;	// the partition mode
   timer			taskTime;	// contains the time used for the last chunk (only used by timed blocked partition)
   double 		speed,totalSpeed,predecessorSpeed;	// speed factors for this loop
         
   inline void	lockBounds() const {int error=SMI_Mutex_lock(mutexBounds);
                                 	assert(error==SMI_SUCCESS);}
   inline void unlockBounds() const {int error=SMI_Mutex_unlock(mutexBounds);
                                 	assert(error==SMI_SUCCESS);}
   inline void	lockBoundsRemote(const int procRank) const
            	{int error=SMI_Mutex_lock(mutexBoundsRemote[procRank]);
               assert(error==SMI_SUCCESS);}
   inline void unlockBoundsRemote(const int procRank) const 
            	{int error=SMI_Mutex_unlock(mutexBoundsRemote[procRank]);
               assert(error==SMI_SUCCESS);}

   void blockedPartition();
   void cyclicPartition();
   void adaptedBlockedPartition();
   void timedBlockedPartition();
   void readHigh();

   void adaptExpo(const int doneMean);
   void adaptLinear(const int doneMean);
   void adaptOpt(const int doneMean);
   
   int helpFirstProc();

   int calcDoneMean();
   void logDone(int chunkSize);
public:
	loop();// throw(err);
	~loop();
   // returns current status
   inline loopStatus currentStatus() {return(status);}  
   void setPartitionMode(const int mode);
   // returns partition mode
   inline int getPartitionMode() {return(partitionMode);}
   void setGlobalBounds(const int gl,const int gh) throw(err);
   void setK(const double kNew) {if(kNew>=1.0)k=kNew;}
   void setMinChunkSizeLocal(const int cs) {if(cs>=1)minChunkSizeLocal=cs;}
   void setMinChunkSizeRemote(const int cs) {if(cs>=1)minChunkSizeRemote=cs;}
   void setMaxChunkSizeLocal(const int cs) {if(cs>=1)maxChunkSizeLocal=cs;}
   void setMaxChunkSizeRemote(const int cs) {if(cs>=1)maxChunkSizeRemote=cs;}
   void setAdaptionMode(const int mode) {if(mode>0)_smi_adaptionMode=mode;}
   void setMaxHelpDist(const int newMaxHelpDist); 
   void setMaxCalcDist(const int newMaxCalcDist); 
   void useProcSpeed();
   void partition();
   void rePartition();
   void getLocalBlocked(int& low,int& high);
   void getLocalCyclic(int& low,int& high);
   void getLocalTimed(int& low,int& high);
   void getRemoteBlocked(int& low,int& high);
   void getRemoteCyclic(int& low,int& high);
   void adaptK();

   // static member functions
   static void initStatic() throw(err);
   static void evaluateSpeed(double* const speedArray) throw(err);
   static int addLoop() throw(err);
   // frees a loop refered to by its id
   inline static void delLoop(const int id) throw(err) {delete loopArray[id];}
   static loop* giveLoop(const int id) throw(err); 
};
#endif


