/**************************************************************************
* VIOLA Parallel IO (ParIO)                                               *
***************************************************************************
*                                                                         *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              *
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         *
* See COPYRIGHT notice in base directory for details                      *
***************************************************************************
***************************************************************************
* File:         pario_probe.c                                             *
* Description:  Thread probing for MPI messages delegating handling to    *
*               other threads.                                            *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"
#include <pthread.h>
#include <assert.h>
#include "pario_threads.h"
#include "pario_probe.h"
#include "ad_memfs.h"
#include "ad_tunnelfs_common.h"

#include "../../../src/tunnelfs_io_server/tunnelfs_log.h"

pthread_t pario_probe;
pthread_mutex_t pario_probe_sync = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_retrieved_from_queue = PTHREAD_COND_INITIALIZER;
int message_is_processing = 0;

int pario_shutdown = 0;

MPI_Comm probe_comm[2];

void *pario_probe_for_msgs(void *args)
{
    int *stop_server = NULL;
    MPI_Status status;
    int flag = 0;
    int spin_i = 0; /* alternates between 0 and 1 */

    stop_server = ((tunnelfs_thread_args_t *) args)->stop_server;
   
    /* setting up communicator structure for probing */
    probe_comm[0] = MPI_COMM_TUNNELFS_WORLD;
    probe_comm[1] = MPI_COMM_MEMFS_WORLD;
   
    /*while (!(*stop_server))*/
    while (!pario_shutdown)
    {
        flag = 0;
        /*
         * If a queue is busy, let's wait till it has been freed.
         */
        pthread_mutex_lock(&pario_probe_sync);
        while (message_is_processing)
        {
            LOG("Waiting for 'message_retrieved_from_queue'");
            pthread_cond_wait(&message_retrieved_from_queue,
                              &pario_probe_sync);
        }
        assert(!message_is_processing); /* sanity check */
        pthread_mutex_unlock(&pario_probe_sync);

        LOG("Probing for message ... ");
        /*
         * Get next message from queue.
         */
        do
        {
            LOCK_MPI();
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, probe_comm[spin_i], &flag, &status);
            UNLOCK_MPI();
            spin_i = (spin_i + 1) % 2;
        }
        /*while (!flag && !(*stop_server));*/
        while (!flag && !pario_shutdown);

        LOG("Got message with tag: 0x%x on comm %i", status.MPI_TAG,
            probe_comm[spin_i]);
        
        /*if (*stop_server)*/
        if (pario_shutdown)
        {
            LOG("Waking up sleeping threads");
            pthread_mutex_lock(&tunnelfs_main_sync);
            /* set status flag to get the thread out of the wait loop */
            tunnelfs_main_status_is_empty = 0;
            pthread_cond_signal(&message_for_tunnelfs_main);
            pthread_mutex_unlock(&tunnelfs_main_sync);

            pthread_mutex_lock(&tunnelfs_service_sync);
            /* set status flag to get the thread out of the wait loop */
            tunnelfs_service_status_is_empty = 0;
            pthread_cond_signal(&message_for_tunnelfs_service);
            pthread_mutex_unlock(&tunnelfs_service_sync);
            LOG("Shutting down probing loop");
            break;
        }

        pthread_mutex_lock(&pario_probe_sync);
        message_is_processing = 1;
        pthread_mutex_unlock(&pario_probe_sync);
            
        if ((status.MPI_TAG >= 0x0000) && (status.MPI_TAG < 0x1000))
        {
            /*
             * Signal main thread to start message processing. Also copy status
             * into local buffer, to be able to probe for another message.
             */
            LOG("Locking tunnelfs main sync mutex");
            pthread_mutex_lock(&tunnelfs_main_sync);
            LOG("done");
            memcpy(&tunnelfs_main_status, &status, sizeof(MPI_Status));
            tunnelfs_main_status_is_empty = 0;
            LOG("Signaling tunnelfs main");
            pthread_cond_signal(&message_for_tunnelfs_main);
            pthread_mutex_unlock(&tunnelfs_main_sync);
        }
        
        if ((status.MPI_TAG >= 0x1000) && (status.MPI_TAG < 0x2000))
        {
            /*
             * Signal service thread to start message processing. Also copy 
             * status into local buffer, to be able to probe for another 
             * message.
             */
            LOG("Locking tunnelfs service sync mutex");
            pthread_mutex_lock(&tunnelfs_service_sync);
            LOG("done");
            memcpy(&tunnelfs_service_status, &status, sizeof(MPI_Status));
            tunnelfs_service_status_is_empty = 0;
            LOG("Signaling tunnelfs service");
            pthread_cond_signal(&message_for_tunnelfs_service);
            pthread_mutex_unlock(&tunnelfs_service_sync);
        }
        
        if ((status.MPI_TAG >= 0x3000) && (status.MPI_TAG < 0x4000))
        {
            /*
             * Signal service thread to start message processing. Also copy 
             * status into local buffer, to be able to probe for another 
             * message.
             */
            LOG("Locking memfs service sync mutex");
            pthread_mutex_lock(&memfs_service_sync);
            LOG("done");
            memcpy(&memfs_service_status, &status, sizeof(MPI_Status));
            memfs_service_status_is_empty = 0;
            LOG("Signaling memfs service");
            pthread_cond_signal(&message_for_memfs_service);
            pthread_mutex_unlock(&memfs_service_sync);
        }
        
        if ((status.MPI_TAG >= 0x4000) && (status.MPI_TAG < 0x5000))
        {
            /*
             * Signal main thread to start message processing. Also copy status
             * into local buffer, to be able to probe for another message.
             */
            LOG("Locking memfs main sync mutex");
            pthread_mutex_lock(&memfs_main_sync);
            LOG("done");
            memcpy(&memfs_main_status, &status, sizeof(MPI_Status));
            memfs_main_status_is_empty = 0;
            LOG("Signaling memfs main");
            pthread_cond_signal(&message_for_memfs_main);
            pthread_mutex_unlock(&memfs_main_sync);
        }
    }
    
    LOG("At end of thread probe");
    pthread_exit(NULL);
    return NULL;
}
