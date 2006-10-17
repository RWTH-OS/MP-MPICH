/**************************************************************************
* TunnelFS Server                                                         * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         tunnelfs_srv.c                                            * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include "mpi.h"
#include "mpio.h"
#include "adio.h"
#include "ad_memfs.h"
#include "ad_tunnelfs.h"
#include "pario_threads.h"
#include "pario_probe.h"
#include "tunnelfs_log.h"
#include "tunnelfs_srv.h"

/**
 * POSIX Thread variable for tunnelfs main thread handling client/server
 * communication
 */
pthread_t tunnelfs_main;
/**
 * POSIX Thread variable for tunnelfs service thread handling server/server
 * communication
 */
pthread_t tunnelfs_service;

/**
 * Structure for keeping track of commandline options for tunnelfs server.
 */
typedef struct
{
    int memfs; /**< If set, the memfs threads are started by the server */
} tunnelfs_srvopt_t;

/**
 * Parse commandline options to tunnelfs server and set options accordingly
 */
int handle_options(int argc, char **argv, tunnelfs_srvopt_t * server_options);
int handle_options(int argc, char **argv, tunnelfs_srvopt_t * server_options)
{
    int i = 0;

    /* TODO:
     * it would be nice to have something like getopt_long working here, but I
     * couldn't get it to work yet. On the other hand, this works fine, too.*/
    for (i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "--disable-memfs", 15) == 0)
            server_options->memfs = 0;
        if (strncmp(argv[i], "--enable-memfs", 14) == 0)
            server_options->memfs = 1;
    }

    return 0;
}

/**
 * Main function of tunnelfs io server creating all threads and sleeping till
 * they are joined.
 * @param argc number of commandline parameter provided in argv
 * @param argv Pointers to strings of commandline parameters
 * @return error code EXIT_SUCCESS
 */
int tunnelfs_srv(int argc, char **argv)
{
    int rcode = 0;

    int status = 0;
    int *stop_server;

    tunnelfs_srvopt_t server_options;

    pthread_attr_t attr;

    tunnelfs_thread_args_t thread_args;

    /* initialize loop variable with 0 */
    stop_server = calloc(1, sizeof(int));
    thread_args.stop_server = stop_server;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_mutex_init(&threadsafe_mpi_mutex, NULL);

    /* Setting default options */
#ifndef DISABLE_MEMFS
    server_options.memfs = 1;
#else
    server_options.memfs = 0;
#endif

    /* Getting user options */
    handle_options(argc, argv, &server_options);

    LOCK_MPI();
    MPI_Init(&argc, &argv);
    UNLOCK_MPI();

    LOG_INIT;
    LOG("starting");

    LOCK_MPI();
    MPI_Comm_size(MPI_COMM_META_REDUCED, &thread_args.num_servers);
    UNLOCK_MPI();

    LOG("%i tunnelfs io server(s) detected", thread_args.num_servers);

    LOCK_MPI();
    MPI_Comm_size(TUNNELFS_COMM_WORLD, &thread_args.num_clients);
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &thread_args.mpi_rank);
    UNLOCK_MPI();

    if (thread_args.mpi_rank == TUNNELFS_GLOBAL_MASTER)
    {
        LOG("Serving as master io node");
        thread_args.num_clients -= thread_args.num_servers;
    }
    else
    {
        LOG("Serving as slave io node");
        thread_args.num_clients = 0;
    }

    tunnelfs_srv_init_serverlist();

    /* start threads */
    LOG("Starting tunnelfs main thread");
    rcode = pthread_create(&tunnelfs_main, &attr, tunnelfs_srv_main,
                           &thread_args);
    if (rcode)
    {
        LOG("Failed to initialize thread");
        exit(-1);
    }

    LOG("Starting tunnelfs service thread");
    rcode = pthread_create(&tunnelfs_service, &attr, tunnelfs_srv_service,
                           &thread_args);
    if (rcode)
    {
        LOG("Failed to initialize thread");
        exit(-1);
    }

    if (server_options.memfs)
    {
        LOG("Starting memfs threads");
        rcode = MEMFS_Init(thread_args.num_servers);
        if (rcode)
        {
            LOG("Failed to initialize threads");
            exit(-1);
        }
    }

    /* Start Probe thread as last, so the others are set up */
    LOG("Starting tunnelfs probe thread");
    rcode = pthread_create(&pario_probe, &attr, pario_probe_for_msgs,
                           &thread_args);
    if (rcode)
    {
        LOG("Failed to initialize thread");
        exit(-1);
    }

    LOG("Server launch complete");

    /* sleep till threads have joined */
    pthread_join(tunnelfs_service, (void **) &status);
    pthread_join(tunnelfs_main, (void **) &status);
    LOG("TUNNELFS threads joined");

    if (server_options.memfs)
    {
        MEMFS_Shutdown();
        LOG("MEMFS threads joined");
    }

#ifdef USE_TUNNELFS_PROBE_THREAD
    pthread_join(pario_probe, (void **) &status);
#endif

    LOG("stopping");

    LOCK_MPI();
    MPI_Finalize();
    UNLOCK_MPI();

    LOG("MPI layer shutdown");

    pthread_mutex_destroy(&threadsafe_mpi_mutex);

    return EXIT_SUCCESS;
}
