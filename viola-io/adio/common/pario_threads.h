/**************************************************************************
* VIOLA Pario MPI multithreading                                          * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
***************************************************************************
***************************************************************************
* File:         pario_threads.h                                           * 
* Description:  Pthread mutex variables to enable threadsafe access to    *
*               MPI function calls                                        *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
**************************************************************************/
#ifndef PARIO_THREADS_H
#define PARIO_THREADS_H     

#include "mpi.h"
#include <pthread.h>
#include <stdio.h>

/**
 * Structure for arguments handed to threads in pthread_create.
 */
typedef struct
{
    int num_servers;    /**< number of io servers. */
    int num_clients;    /**< number of io clients. */
    int mpi_rank;       /**< rank, in respect to TUNNELFS_COMM_WORLD */
    int *stop_server;   /**< pointer on global variable to stop threads */
}
tunnelfs_thread_args_t;

extern pthread_t *pario_mpi_lock_owner;

#ifdef LOCK_LOGGING
/**
 * Log start of optaining the lock for MPI
 */
#define LL_START fprintf(stderr, "[%2i] LOCK (%s, Line %i)\n", tunnelfs_log_rank,  __FILE__, __LINE__);
/**
 * Log release of lock for MPI
 */
#define LL_STOP  fprintf(stderr, "[%2i] UNLOCK (%s, Line %i)\n", tunnelfs_log_rank, __FILE__, __LINE__);
/**
 * Sanity Check before locking
 */
#define LL_CHECK if (pario_mpi_lock_owner == tunnelfs_log_rank) \
    fprintf(stderr, "[%2i] Preparing to die in DEADLOCK (%s, Line %i)!\n", tunnelfs_log_rank, __FILE__, __LINE__); 
/**
 * Reset sanity check before unlocking
 */
#define LL_CHECK_RESET {pario_mpi_lock_owner = -1;}
#else
/**
 * Empty macro as logging for the mpi lock is disabled 
 */
#define LL_START
/**
 * Empty macro as logging for the mpi lock is disabled 
 */
#define LL_STOP
#define LL_CHECK
#define LL_CHECK_RESET
#endif

/***************************************************************************/

#ifdef LOCK_TIMING
/**
 * Start of timing for optaining the lock for MPI
 */
#define LT_START {pt_start = ptWtime();}
/**
 * End of timing for obtaining the lock for MPI and adding duration to overall
 * lock waiting time.
 */
#define LT_STOP  {pt_stop = ptWtime(); \
                 pario_lock_time += pt_stop - pt_start;}
#else
/**
 * Empty macro as logging for the mpi lock is disabled 
 */
#define LT_START
/**
 * Empty macro as logging for the mpi lock is disabled 
 */
#define LT_STOP
#endif

/***************************************************************************/

/**
 * Locking MPI for exclusive usage. Enables threadsafe use of MPI functions
 */
#define LOCK_MPI()      {LL_START \
                        LT_START \
                        pthread_mutex_lock(&threadsafe_mpi_mutex); \
                        LT_STOP;}
/**
 * Unlocking MPI after exclusive usage. Enables threadsafe use of MPI functions
 */
#define UNLOCK_MPI()    {LL_STOP \
                        pthread_mutex_unlock(&threadsafe_mpi_mutex);}

/***************************************************************************/

/**
 * POSIX threads mutex variable for exclusive use of MPI with multiple threads
 */
extern pthread_mutex_t threadsafe_mpi_mutex;

#ifdef LOCK_TIMING
/**
 * start of optaining a lock on MPI
 */
extern double pt_start;
/**
 * end of optaining a lock on MPI
 */
extern double pt_stop;
/**
 * overall time spent waiting for an MPI lock
 */
extern double pario_lock_time;
#endif

/**
 * Obtain the current wall clock time
 * @return The current wall clock time
 */
double ptWtime();

/**
 * An implementation of MPI_Send using immediate send and test to prevent
 * deadlocking of several threads using MPI at the same time.
 * @param buf The send buffer
 * @param count The number of elements
 * @param datatype The type of elements
 * @param dest The destination rank
 * @param tag The message tag
 * @param comm The communicator used
 * @return MPI errorcode
 */
int ptMPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
               MPI_Comm comm);

/**
 * An implementation of MPI_Recv using immediate receive and test to prevent
 * deadlocking of several threads using MPI at the same time.
 * @param buf The receive buffer
 * @param count The number of elements
 * @param datatype The type of elements
 * @param source The source rank
 * @param tag The message tag
 * @param comm The communicator used
 * @param status The MPI status structure containing information about the
 *        receive
 * @return MPI errorcode
 */
int ptMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source,
               int tag, MPI_Comm comm, MPI_Status *status);

/**
 * An implementation of MPI_Probe using immediate probe and test to prevent
 * deadlocking of several threads using MPI at the same time.
 * @param source The source rank
 * @param tag The message tag
 * @param comm The communicator used
 * @param status The MPI status structure containing information about the
 *        probe
 * @return MPI errorcode
 */
int ptMPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status);
#endif
