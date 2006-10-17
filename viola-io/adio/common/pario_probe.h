/**************************************************************************
* TunnelFS IO Server                                                      * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
***************************************************************************
***************************************************************************
* File:         tunnelfs_srv_threads.h                                    * 
* Description:  Header file for tunnelfs thread handling                  *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
**************************************************************************/
#ifndef _PARIO_PROBE_H
#define _PARIO_PROBE_H

#include "mpi.h"

/**
 * POSIX Thread variable for probing thread
 */
extern pthread_t pario_probe;
/**
 * Indicator of ongoing message processing
 */
extern int message_is_processing;

/** 
 * Probing for incomming message using MPI.
 */
void *pario_probe_for_msgs(void *args);
/**
 * Mutex of thread synchronization with other threads.
 */
extern pthread_mutex_t pario_probe_sync;
/**
 * Condition variable if old message has been received.
 */
extern pthread_cond_t message_retrieved_from_queue;
/**
 * Flag indicating shutdown of all pario threads
 */
extern int pario_shutdown;
/** 
 * Prototype of function given to pthread_create.
 */
void *tunnelfs_srv_main(void *args);
/** 
 * Mutex of thread synchronization with probe thread.
 */
extern pthread_mutex_t tunnelfs_main_sync;
/** 
 * Condition variable for thread synchronization with probe thread.
 */
extern pthread_cond_t message_for_tunnelfs_main;
/** 
 * Status that is used in tunnelfs main thread.
 */
extern MPI_Status tunnelfs_main_status;
/**
 * Status modification indicator
 */
extern int tunnelfs_main_status_is_empty;

/** 
 * Prototype of function given to pthread_create.
 */
void *tunnelfs_srv_service(void *args);
/** 
 * Mutex of thread synchronization with probe thread.
 */
extern pthread_mutex_t tunnelfs_service_sync;
/** 
 * Condition variable for thread synchronization with probe thread.
 */
extern pthread_cond_t message_for_tunnelfs_service;
/** 
 * Status that is used in tunnelfs service thread.
 */
extern MPI_Status tunnelfs_service_status;
/**
 * Status modification indicator
 */
extern int tunnelfs_service_status_is_empty;

/** 
 * Mutex of thread synchonization with probe thread.
 */
extern pthread_mutex_t memfs_main_sync;
/** 
 * Condition variable for thread synchonization with probe thread.
 */
extern pthread_cond_t message_for_memfs_main;
/** 
 * Status that is used in memfs main thread.
 */
extern MPI_Status memfs_main_status;
/**
 * Status modification indicator
 */
extern int memfs_main_status_is_empty;

/** 
 * Mutex of thread synchonization with probe thread.
 */
extern pthread_mutex_t memfs_service_sync;
/** 
 * Condition variable for thread synchonization with probe thread.
 */
extern pthread_cond_t message_for_memfs_service;
/** 
 * Status that is used in memfs service thread.
 */
extern MPI_Status memfs_service_status;
/**
 * Status modification indicator
 */
extern int memfs_service_status_is_empty;

#endif
