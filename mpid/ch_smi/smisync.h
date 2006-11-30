#ifndef _MPID_SMI_SYNC_H
#define _MPID_SMI_SYNC_H

/* $Id$ */
/* macros for memcpy-synchronization between processes */

#include "smidef.h"
#include "smistat.h"
#include "smidebug.h"

#define MEMCPYSYNC_ENTER(_dest, _l) if (MPID_SMI_is_remote[_dest] \
                                            && (MPID_SMI_cfg.MEMCPYSYNC_MODE != MEMCPYSYNC_NONE) \
                                            && ((_l) > MPID_SMI_cfg.MEMCPYSYNC_MIN)) { \
	/* which locks do we need ? */ \
	int have_out_lock, have_in_lock; \
	have_out_lock = !(MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_OUT); /* outgoing sync (local node) */\
	have_in_lock  = !(MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_IN); /* ingoing sync (remote node) */\
	\
	while (!(have_out_lock && have_in_lock)) { \
	    /* try to get the locks we need */ \
	    if (!have_out_lock) \
		SMI_Mutex_trylock(MPID_SMI_memlocks_out[MPID_SMI_myNode], &have_out_lock); \
	    if (!have_in_lock) \
		SMI_Mutex_trylock(MPID_SMI_memlocks_in[MPID_SMI_procNode[_dest]], &have_in_lock); \
	    \
	    /* if we don't have all locks we need, do something useful */ \
	    if (!(have_out_lock && have_in_lock)) { \
		if (!have_in_lock) \
		    MPID_STAT_COUNT(no_incpy_lock); \
		if (!have_out_lock) \
		    MPID_STAT_COUNT(no_outcpy_lock); \
                \
		if (have_out_lock && (MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_OUT)) { \
		    SMI_Mutex_unlock(MPID_SMI_memlocks_out[MPID_SMI_myNode]); \
		    have_out_lock = 0; \
		} \
		if (have_in_lock && (MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_IN)) { \
		    SMI_Mutex_unlock(MPID_SMI_memlocks_in[MPID_SMI_procNode[_dest]]); \
		    have_in_lock = 0; \
		} \
		MPID_SMI_DEBUG_PRINT_MSG("Didn't get access to memory locks - checking device"); \
		MPID_DeviceCheck(MPID_NOTBLOCKING); \
	    } \
	} \
    } 

#define MEMCPYSYNC_ENTER_NODEVCHECK(_dest, _l) if (MPID_SMI_is_remote[_dest] \
                                         && (MPID_SMI_cfg.MEMCPYSYNC_MODE != MEMCPYSYNC_NONE) \
                                         && ((_l) > MPID_SMI_cfg.MEMCPYSYNC_MIN)) { \
	/* which locks do we need ? */ \
	int have_out_lock, have_in_lock; \
	have_out_lock = !(MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_OUT); /* outgoing sync (local node) */\
	have_in_lock  = !(MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_IN); /* ingoing sync (remote node) */\
	\
	while (!(have_out_lock && have_in_lock)) { \
	    /* try to get the locks we need */ \
	    if (!have_out_lock) \
		SMI_Mutex_trylock(MPID_SMI_memlocks_out[MPID_SMI_myNode], &have_out_lock); \
	    if (!have_in_lock) \
		SMI_Mutex_trylock(MPID_SMI_memlocks_in[MPID_SMI_procNode[_dest]], &have_in_lock); \
	    \
	    /* if we don't have all locks we need, do something useful */ \
	    if (!(have_out_lock && have_in_lock)) { \
		if (!have_in_lock) \
		    MPID_STAT_COUNT(no_incpy_lock); \
		if (!have_out_lock) \
		    MPID_STAT_COUNT(no_outcpy_lock); \
                \
		if (have_out_lock && (MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_OUT)) { \
		    SMI_Mutex_unlock(MPID_SMI_memlocks_out[MPID_SMI_myNode]); \
		    have_out_lock = 0; \
		} \
		if (have_in_lock && (MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_IN)) { \
		    SMI_Mutex_unlock(MPID_SMI_memlocks_in[MPID_SMI_procNode[_dest]]); \
		    have_in_lock = 0; \
		} \
	    } \
	} \
    } 


#define MEMCPYSYNC_LEAVE(_dest, _l)   if (MPID_SMI_is_remote[_dest] \
                                              && (MPID_SMI_cfg.MEMCPYSYNC_MODE != MEMCPYSYNC_NONE) \
                                              && ((_l) > MPID_SMI_cfg.MEMCPYSYNC_MIN)) { \
	/* which locks do we own ? */ \
	if (MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_OUT) \
	    SMI_Mutex_unlock(MPID_SMI_memlocks_out[MPID_SMI_myNode]); \
	if (MPID_SMI_cfg.MEMCPYSYNC_MODE & MEMCPYSYNC_IN) \
	    SMI_Mutex_unlock(MPID_SMI_memlocks_in[MPID_SMI_procNode[_dest]]); \
    } 


#endif
