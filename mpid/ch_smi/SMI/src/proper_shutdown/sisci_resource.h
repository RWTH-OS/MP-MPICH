/* $Id: sisci_resource.h,v 1.1 2004/03/19 22:14:19 joachim Exp $ */

/* This header is used to generally switch any sisci calls to the resourcelist
 * controlled calls */

#ifndef __SMI_SISCI_RESOURCE_H__
#define __SMI_SISCI_RESOURCE_H__

#include <sisci_api.h>

#ifdef SCIOpen
#undef SCIOpen
#endif
#define SCIOpen(sd,flags,error) \
  rs_SCIOpen((sd),(flags),(error))

#ifdef SCIClose
#undef SCIClose
#endif
#define SCIClose(sd,flags,error) \
  rs_SCIClose((sd),(flags),(error))

#ifdef SCICreateSegment
#undef SCICreateSegment
#endif
#define SCICreateSegment(sd,segment,segmentId,size,callback,callbackArg,flags,error)\
  rs_SCICreateSegment((sd),(segment),(segmentId),(size),(callback),(callbackArg),(flags),(error))

#ifdef SCIConnectSegment
#undef SCIConnectSegment
#endif
#define SCIConnectSegment(sd,segment,nodeId,segmentId,localAdapterNo,callback,callbackArg,timeout,flags,error) \
  rs_SCIConnectSegment((sd),(segment),(nodeId),(segmentId),(localAdapterNo),(callback),(callbackArg),(timeout),(flags),(error))

#ifdef SCIMapLocalSegment 
#undef SCIMapLocalSegment  
#endif
#define SCIMapLocalSegment(sd,map,offset,size,addr,flags,error) \
  rs_SCIMapLocalSegment((sd),(map),(offset),(size),(addr),(flags),(error))

#ifdef SCIMapRemoteSegment  
#undef SCIMapRemoteSegment   
#endif
#define SCIMapRemoteSegment(sd,map,offset,size,addr,flags,error) \
  rs_SCIMapRemoteSegment((sd),(map),(offset),(size),(addr),(flags),(error))

#ifdef SCIUnmapSegment   
#undef SCIUnmapSegment    
#endif
#define SCIUnmapSegment(map,flags,error) \
  rs_SCIUnmapSegment((map),(flags),(error))

#ifdef SCIDisconnectSegment     
#undef SCIDisconnectSegment     
#endif
#define SCIDisconnectSegment(sd,flags,error)  \
  rs_SCIDisconnectSegment((sd),(flags),(error))

#ifdef SCIRemoveSegment      
#undef SCIRemoveSegment      
#endif
#define SCIRemoveSegment(sd,flags,error) \
  rs_SCIRemoveSegment((sd),(flags),(error))

#ifdef SCICreateMapSequence       
#undef SCICreateMapSequence       
#endif
#define SCICreateMapSequence(map,sequence,flags,error) \
  rs_SCICreateMapSequence((map),(sequence),(flags),(error))

#ifdef SCIRemoveSequence         
#undef SCIRemoveSequence        
#endif
#define SCIRemoveSequence(sequence,flags,error) \
  rs_SCIRemoveSequence((sequence),(flags),(error))

#ifdef SCICreateDMAQueue          
#undef SCICreateDMAQueue         
#endif
#define SCICreateDMAQueue(sd,dq,localAdapterNo,maxEntries,flags,error) \
  rs_SCICreateDMAQueue((sd),(dq),(localAdapterNo),(maxEntries),(flags),(error))

#ifdef SCIRemoveDMAQueue           
#undef SCIRemoveDMAQueue          
#endif
#define SCIRemoveDMAQueue(dq,flags,error) \
  rs_SCIRemoveDMAQueue((dq),(flags),(error))

#ifdef SCICreateInterrupt            
#undef SCICreateInterrupt           
#endif
#define SCICreateInterrupt(sd,interrupt,localAdapterNo,interruptNo,callback,callbackArg,flags,error) \
  rs_SCICreateInterrupt((sd),(interrupt),(localAdapterNo),(interruptNo),(callback),(callbackArg),(flags),(error))

#ifdef SCIRemoveInterrupt             
#undef SCIRemoveInterrupt            
#endif
#define SCIRemoveInterrupt(interrupt,flags,error) \
  rs_SCIRemoveInterrupt((interrupt),(flags),(error)) \

#ifdef SCIConnectInterrupt              
#undef SCIConnectInterrupt             
#endif
#define SCIConnectInterrupt(sd,interrupt,nodeId,localAdapterNo,interruptNo,timeout,flags,error) \
  rs_SCIConnectInterrupt((sd),(interrupt),(nodeId),(localAdapterNo),(interruptNo),(timeout),(flags),(error))

#ifdef SCIDisconnectInterrupt               
#undef SCIDisconnectInterrupt              
#endif
#define SCIDisconnectInterrupt(interrupt,flags,error) \
  rs_SCIDisconnectInterrupt((interrupt),(flags),(error))

#endif  /* __SMI_SISCI_RESOURCE_H__ */



