/* $Id: sync.h,v 1.1 2004/03/19 22:14:23 joachim Exp $ */

#ifndef _SMI_SYNC_H_
#define _SMI_SYNC_H_

#include "env/general_definitions.h"

/* Enable the use of Schulz locks and barriers                                   */ 
/* #define USE_SCHULZSYNC */

/* Use modificatiuons that allow to define barriers for a special set of         */
/* prozesses 						       		         */
#define ALLOW_SYNCHRONIZATION_SET 0

/* This patch is needed since syncmod requires the LINUX define */
#ifdef USE_SCHULZSYNC
#define USED_WITHIN_SMI
#ifndef LINUX
#define LINUX
#include "syncmod.h"
#undef LINUX 
#else
#include "syncmod.h"
#endif 
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************/
/*** This data structure contains all necessary mutex informations.            ***/
/*********************************************************************************/
typedef struct mutex_id
{
    volatile int *shmadr;
    int algorithm_type, home_of_data, high;
#ifdef USE_SCHULZSYNC
    syncMod_atomic_t atomic; 				/* used for Schulz Locks */
#endif
} mutex_id;
 
/*********************************************************************************/
/*** This data structures contains all necessary barrier informations.         ***/
/*********************************************************************************/

typedef struct barrier_id
{
    int algorithm_type;
    int progress_counter_id;

#if ALLOW_SYNCHRONIZATION_SET
    int *process_ranks;
    int nbr_processes;
    int is_active;
#endif

#ifdef USE_SCHULZSYNC
    syncMod_atomic_t atomic; 		             /* used for Schulz Barriers */
#endif

} barrier_id;



#ifdef __cplusplus
}
#endif


#endif /* _SYNC_H_ */
