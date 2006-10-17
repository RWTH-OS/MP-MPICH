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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"
#include <pthread.h>
#include "pario_threads.h"
#include <sys/time.h>
#include "../../../src/tunnelfs_io_server/tunnelfs_log.h"

pthread_mutex_t threadsafe_mpi_mutex = PTHREAD_MUTEX_INITIALIZER;
int threadsafe_mpi_mutex_initialized = 1;

/**
 * start time of optaining a lock on MPI
 */
double pt_start = 0;
/**
 * end time of optaining a lock on MPI
 */
double pt_stop = 0;
/**
 * overall time spent waiting for an MPI lock
 */
double pario_lock_time = 0;

/**
 * Obtain the current wall clock time
 * @return The current wall clock time
 */
double ptWtime()
{
    struct timeval t;

    gettimeofday(&t, NULL);
    return (t.tv_sec + t.tv_usec*1e-6);
}

/**
 * thread id that owns the mpi lock
 */
pthread_t *pario_mpi_lock_owner = NULL;

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
               MPI_Comm comm)
{
    MPI_Request req;
    MPI_Status stat;
    int flag = 0;
    int tunnelfs_log_rank = -1;

    if (!threadsafe_mpi_mutex_initialized)
    {
        pthread_mutex_init(&threadsafe_mpi_mutex, NULL);
        threadsafe_mpi_mutex_initialized = 1;
    }

    LOCK_MPI();
    MPI_Isend(buf, count, datatype, dest, tag, comm, &req);
    UNLOCK_MPI();
    do
    {
        flag = 0;
        LOCK_MPI();
        MPI_Test(&req, &flag, &stat);
        UNLOCK_MPI();
    }
    while (flag != 1);
}

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
               int tag, MPI_Comm comm, MPI_Status *status)
{
    MPI_Request req;
    int flag = 0;
    int internal_count;
    int dtype_size;

    if (!threadsafe_mpi_mutex_initialized)
    {
        pthread_mutex_init(&threadsafe_mpi_mutex, NULL);
        threadsafe_mpi_mutex_initialized = 1;
    }

    do
    {
        flag = 0;
        LOCK_MPI();
        MPI_Iprobe(source, tag, comm, &flag, status);
        if (flag)
            MPI_Recv(buf, count, datatype, source, tag, comm,
                     status);
        UNLOCK_MPI();
    }
    while (flag != 1);
}

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
int ptMPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    int flag = 0;

    if (!threadsafe_mpi_mutex_initialized)
    {
        pthread_mutex_init(&threadsafe_mpi_mutex, NULL);
        threadsafe_mpi_mutex_initialized = 1;
    }

    do
    {
        flag = 0;
        LOCK_MPI();
        MPI_Iprobe(source, tag, comm, &flag, status);
        UNLOCK_MPI();
    }
    while (flag != 1);
}
