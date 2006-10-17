/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_main.c                                           * 
* Description:  This is the "memfs main thread"                           * 
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/  

/*===================================================================================================*/
/* INCLUDES */

#include "ad_memfs_main.h"
#include "ad_tunnelfs_msg.h"
#include "ad_tunnelfs_common.h"
#include "tunnelfs_srv_sync.h"
#include "ad_memfs_lock.h"
#include "ad_memfs_files.h"
#include "pario_probe.h"
#include "pario_map.h"
/*#include "tunnelfs_srv.h"*/

/*===================================================================================================*/
/* DEFINES */

/*===================================================================================================*/
/* TYPEDEFS */

/*===================================================================================================*/
/* VARIABLES */

int recv_buf_size = 0;
void *recv_buf = NULL;
int send_buf_size = 0;
void *send_buf = NULL;
int iodata_buf_size = 0;
void *iodata_buf = NULL;

int msg_id = 0;
int main_msg_id = 0;

int comm_size, comm_rank, globalrank, memfs_num_servers;

int *serverlist = NULL;
int listcount = 0;

#ifdef MEMFS_TIME
double t1, t2, totaltime1, totaltime2;
#endif

pthread_mutex_t memfs_main_sync = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_for_memfs_main = PTHREAD_COND_INITIALIZER;
MPI_Status memfs_main_status;
int memfs_main_status_is_empty = 1;


/*===================================================================================================*/
/* STATIC FUNCTIONS */


/**
 * Function performs operations necessary for communication between
 * memfs_main thread and memfs ADIO device function.
 * Communication is done with locks on shared memory segments
 */
void *thread_comm_io(int function, ADIO_File fd, char *buffer, int *error_code, 
                    int param, MPI_Datatype datatype, int file_ptr_type,
                    ADIO_Offset offset, ADIO_Status *status, ADIO_Fcntl_t *fcntl_struct, void *extraParam)
{
    int error;
    static ADIO_Offset off;

    /* lock the mutex variable */
    error = pthread_mutex_lock(&mutex);
    while(io_req.function != 0)
        pthread_cond_wait(&cond_new_call, &mutex); /* wait until new function may be issued to memfs_main */

    /* set values of shared memory */
    io_req.function = function; /* identifies actual device function */
    io_req.fd = fd;
    if(buffer != NULL) /* only necessary for functions that actually use the buffer */
        io_req.buffer = buffer;
    io_req.param = param;
    io_req.datatype = datatype;
    io_req.file_ptr_type = file_ptr_type;
    io_req.offset = offset;
    io_req.status = status;
    io_req.fcntl_struct = fcntl_struct;
   
    if(io_req.function == MEMFS_SETINFO) {
        io_req.extraParam = extraParam;
    }

    pthread_cond_broadcast(&cond_req_set); /* new request is set, broadcast to memfs_main */

    /* next two lines maybe can be removed */
    /* maybe second mutex variable useful / necessary */
    pthread_mutex_unlock(&mutex);
    error = pthread_mutex_lock(&mutex);

    while(io_req.function != 0) {
        /* wait until memfs_main finished handling of function */
        pthread_cond_wait(&cond_finish_call, &mutex);
    } 

     /* get values from shared memory */
    fd = io_req.fd;
    if(buffer != NULL)
        buffer = io_req.buffer;

    error = io_req.error;
    *error_code = error;

    if(io_req.function == MEMFS_SEEKIND) {
        off = io_req.offset;
    }

    io_req.function = 0;
    io_req.error = 0;
   
    /* handling of function finished */
    /* signal that new function may be issued to memfs_main */
    pthread_cond_broadcast(&cond_new_call);
    /* release the mutex lock */
    pthread_mutex_unlock(&mutex);

    if(io_req.function == MEMFS_SEEKIND) {
        /* TODO: THIS MAY RESULT IN AN ERROR */
        return (void *) &off;
    }

    return NULL;
}


/**
 * Same function as above with less parameters. Called by ADIO_MEMFS_Open for example
 */
void thread_comm(int function, ADIO_File fd, char *buffer, int *error_code)
{
    thread_comm_io(function, fd, buffer, error_code, 0, (MPI_Datatype) NULL, 
                   0, (ADIO_Offset) NULL, NULL, NULL, NULL);
}


/*===================================================================================================*/
/* NON-STATIC FUNCTIONS */


/**
 * Initialization of MEMFS Server
 */
int MEMFS_Init(int num_servers) {
    memfs_param_t memfs_thread_param; /* now NOT in ad_memfs.h */
    pthread_attr_t attr;
    io_req.function = 0;

    if(!threads_initialized) {
        memfs_init_threads();

        memfs_thread_param.num_servers = num_servers;
#ifdef DEBUG_THREADS
        fprintf(stderr, "memfs_thread_param.num_servers: %d\n", memfs_thread_param.num_servers);
#endif
        memfs_thread_param.master_server = 0;
        memfs_num_servers = num_servers;        

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_create(&thrMain, &attr, memfs_main_loop, NULL);
        if(num_servers > 1)
            pthread_create(&thrService, &attr, memfs_service, &memfs_thread_param);
        threads_initialized = 1;
    }
    pthread_attr_destroy(&attr);
#ifdef MEMFS_TIME
    if(!timeInitialized()) {
        initTimeMeasurement();
    }
#endif
     
    return 0;
}


/**
 * Shutdown of MEMFS Server
 */
int MEMFS_Shutdown() {
    int rc, status;

//#ifdef DEBUG_THREADS
    fprintf(stderr, "Shutting down memfs threads now!\n");
//#endif
    pthread_mutex_lock(&shutdown_mutex);
    memfs_shutdown = 1;
    pthread_mutex_unlock(&shutdown_mutex);

    fprintf(stderr, "Shutting down io req!\n");
    pthread_mutex_lock(&mutex);
    io_req.function = -1;
    pthread_mutex_unlock(&mutex);
    pthread_cond_broadcast(&cond_req_set);

    pario_shutdown = 1;
    fprintf(stderr, "Shutting down memfs main thread now!\n");
    pthread_mutex_lock(&memfs_main_sync);
    pthread_cond_signal(&message_for_memfs_main);
    pthread_mutex_unlock(&memfs_main_sync);

    fprintf(stderr, "Shutting down memfs service thread now!\n");
    pthread_mutex_lock(&memfs_service_sync);
    pthread_cond_signal(&message_for_memfs_service);
    pthread_mutex_unlock(&memfs_service_sync);
    
    fprintf(stderr, "Waiting for main join\n");
    rc = pthread_join(thrMain, (void **)&status);
  
    fprintf(stderr, "Waiting for service join\n");
    if(memfs_num_servers >1)
        rc = pthread_join(thrService, (void **)&status);
    
    fprintf(stderr, "Destroying thread structures\n");
    memfs_destroy_threads();
    return 0;
}

/**
 * Init mutex and condition variables
 */
int memfs_init_threads() {
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&shutdown_mutex, NULL);

    pthread_mutex_init(&filesystem_mutex, NULL);

    pthread_cond_init(&cond_new_call, NULL);
    pthread_cond_init(&cond_finish_call, NULL);
    pthread_cond_init(&cond_req_set, NULL);

    return 0;
}

/**
 * When finished destroy (free) mutex and condition variables
 */
int memfs_destroy_threads() {
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&shutdown_mutex);

    pthread_mutex_destroy(&filesystem_mutex);

    pthread_cond_destroy(&cond_new_call);
    pthread_cond_destroy(&cond_finish_call);
    pthread_cond_destroy(&cond_req_set);
    return 0;
}

/**
 * function that computes a server for a specific client
 * called at read and write operations 
 * mode 0 = read, contains offset and count because function is called before actual read is performed
 * mode 1 = write, offset and count not necessary because function is called after actual write was performed
 */
void memfs_get_server_for_client(int mode, int file_id, int client_rank, MPI_Offset offset, int count, int *server_rank) {
    int local_rank;
    int etype_size = 4;
    int global;

    local_rank = (offset * etype_size / MEMFS_BLOCKSIZE) % memfs_num_servers;
    global = pario_map_r2w(local_rank);

#ifdef DEBUG_SERVERMAPPING
    fprintf(stderr, "[m%d]: memfs_get_server_for_client mode: %d\n", comm_rank, mode);
    fprintf(stderr, "[m%d]: file %d, client %d, offset %lld, count %d. setting server to %d (global: %d)\n",
            comm_rank, file_id, client_rank, offset, count, local_rank, global);
#endif

    /* *server_rank = global; */
    *server_rank = globalrank;
}


/**
 * Function to set memfs fileserver distribution for a client list
 * comm_ranks is an array with the ranks of the clients within the client communicator
 * client_comm_size gives the number of clients involved
 * returns list which is a mapping of client -> fileserver
 */
void memfs_get_fileserver_distribution(int* comm_ranks, int client_comm_size, dist_list_t* list) {
#ifdef DEBUG_THREADS
    int i;
#endif
/*    memfs_globalmaster_distribution(comm_ranks, client_comm_size, list); */
    memfs_roundrobin_distribution(comm_ranks, client_comm_size, list);
#ifdef DEBUG_THREADS
    for(i = 0; i < client_comm_size; i++) {
        fprintf(stderr, "client %d gets server %d\n", list[i].client, list[i].fileserver);
    }
#endif

}

/**
 * memfs fileserver distribution where the TUNNELFS_GLOBAL_MASTER server is assigned to each client
 */
int memfs_globalmaster_distribution(int* comm_ranks, int client_comm_size, dist_list_t* list) {
    int i;
    for(i = 0; i < client_comm_size; i++) {
        list[i].client = comm_ranks[i];
        list[i].fileserver = TUNNELFS_GLOBAL_MASTER;
    }
    return 0;
}


/**
 * memfs fileserver roundrobin distribution
 * distribution begins where last distribution ended (i is static)
 * this is done for even distribution of fileservers (doesn't always start at fileserver 0)
 */
int memfs_roundrobin_distribution(int* comm_ranks, int client_comm_size, dist_list_t* list) {
    static int i = 0;
    int j;
    for(j = 0; j < client_comm_size; j++) {
        list[j].client = comm_ranks[j];
        list[j].fileserver = serverlist[i];
        i = (i + 1) % listcount;
    }
    return 0;
}


/**
 * Wait for reply of other Memfs Server
 * Only accept messages with the a value given as a parameter to the function
 * Receive reply in buffer recv_buf
 */
int get_reply(MPI_Status *msg_status, int *msg_size, int tag) {
    int flag;
    int wait_for_reply = 1;

#ifndef USE_TUNNELFS_PROBE_THREAD
    while(wait_for_reply) {
        LOCK_MPI();
        MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_MEMFS_WORLD, &flag, msg_status);
/*        if(flag && msg_status->MPI_TAG >= 0x4000 && msg_status->MPI_TAG <= 0x4fff) */
        if(flag && msg_status->MPI_TAG == tag)
        {
#ifdef DEBUG_THREADS
            fprintf(stderr, "[m%d]: reply received from rank %d with tag: %d\n", 
                    comm_rank, msg_status->MPI_SOURCE, msg_status->MPI_TAG);
#endif
            /* Retrieving size of next message */
            /* Messages of type MEMFS_REPLY_IODATA are received as MPI_BYTE */
            /* All other messages are received as MPI_PACKED and then unpacked */
            if(tag == MEMFS_REPLY_IODATA)
                MPI_Get_count(msg_status, MPI_BYTE, msg_size);
            else
                MPI_Get_count(msg_status, MPI_PACKED, msg_size);
            UNLOCK_MPI();
            tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, *msg_size);
            if(tag == MEMFS_REPLY_IODATA)
                ptMPI_Recv(recv_buf, recv_buf_size, MPI_BYTE,
                           msg_status->MPI_SOURCE, msg_status->MPI_TAG,
                           MPI_COMM_MEMFS_WORLD, msg_status);
           else
                ptMPI_Recv(recv_buf, recv_buf_size, MPI_PACKED,
                           msg_status->MPI_SOURCE, msg_status->MPI_TAG,
                           MPI_COMM_MEMFS_WORLD, msg_status);
            wait_for_reply = 0;
         } else {
            UNLOCK_MPI();
         }
    }
#else

        /* initially locking mutex */
        pthread_mutex_lock(&memfs_main_sync);

        /* clearing the status field */
        memset(&memfs_main_status, 0, sizeof(MPI_Status));
        memfs_main_status_is_empty = 1;

        /* wait for message signal */
        while ((memfs_main_status_is_empty) && !pario_shutdown)
            pthread_cond_wait(&message_for_memfs_main,
                              &memfs_main_sync);

        /* if thread is awakend for shutting down, exit! */
        /* TODO: if shutdown is signaled, while the program is waiting for a
         * message, something is fishy! */
        if (pario_shutdown)
            pthread_exit((void *) 0);

#ifdef DEBUG_THREADS
        fprintf(stderr, "[m%d]: Received message with tag: %d from: %d\n", 
                comm_rank, memfs_main_status.MPI_TAG, memfs_main_status.MPI_SOURCE);
#endif
        if(memfs_main_status.MPI_TAG != tag) fprintf(stderr, "[m%d]: WARNING: Required tag: %d\n", comm_rank, tag);

        if (memfs_main_status.MPI_TAG != MEMFS_REPLY_IODATA)
        {
            LOCK_MPI();
            MPI_Get_count(&memfs_main_status, MPI_PACKED, msg_size);

            tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, *msg_size);

            MPI_Recv(recv_buf, *msg_size, MPI_PACKED,
                     memfs_main_status.MPI_SOURCE,
                     memfs_main_status.MPI_TAG, MPI_COMM_MEMFS_WORLD,
                     msg_status);
            UNLOCK_MPI();
         } else {
            LOCK_MPI();
            MPI_Get_count(&memfs_main_status, MPI_BYTE, msg_size);

            tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, *msg_size);

            MPI_Recv(recv_buf, *msg_size, MPI_BYTE,
                     memfs_main_status.MPI_SOURCE,
                     memfs_main_status.MPI_TAG, MPI_COMM_MEMFS_WORLD,
                     msg_status);
            UNLOCK_MPI();

         }

            /**
             * Message is retrieved. Let's signal this to probe thread.
             */
            pthread_mutex_lock(&pario_probe_sync);
            message_is_processing = 0;
            pthread_cond_signal(&message_retrieved_from_queue);
            pthread_mutex_unlock(&pario_probe_sync);

        /* INFO: IO Data is received in default case statement */

#endif
    return 0;
}


/**
 * Main loop which starts when memfs_main thread is created.
 * Handle requests from memfs ADIO device.
 */
void *memfs_main_loop() {
    int error;
    int i_am_master, master_server;
    int dest; /* destination server of a request in TUNNELFS_COMM_WORLD */
 
    MPI_Comm_size(MPI_COMM_MEMFS_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_MEMFS_WORLD, &comm_rank);
    MPI_Comm_rank(TUNNELFS_COMM_WORLD, &globalrank);

    /* Select one master server */
    master_server = 0;
    if(comm_rank == master_server) 
        i_am_master = 1;
    else
        i_am_master = 0;

   tunnelfs_srv_get_serverlist(&serverlist, &listcount);

#ifndef USE_TUNNELFS_PROBE_THREAD
fprintf(stderr, "NOT USING TUNNELFS_PROBE_THREAD\n");
#else
fprintf(stderr, "USING TUNNELFS_PROBE_THREAD\n");
#endif 

#ifdef DEBUG_THREADS 
    fprintf(stderr, "memfs_main_loop() is running with %d Servers overall! rank: %d, global rank: %d\n", 
            comm_size, comm_rank, globalrank); 
#endif
    if(comm_rank == 0) fprintf(stderr, "MEMFS_BLOCKSIZE: %d\n", MEMFS_BLOCKSIZE);
 
    /* function handling is done until memfs_main is stopped */
    while(1)
    {
#ifdef DEBUG_THREADS
    fprintf(stderr, "[m%d]: waiting for request\n", comm_rank);
#endif
      error = pthread_mutex_lock(&mutex);
      while(io_req.function == 0)
          pthread_cond_wait(&cond_req_set, &mutex); /* wait until a request is set */

#ifdef DEBUG_THREADS
    fprintf(stderr, "[m%d]: handling request: %d\n",
            comm_rank, io_req.function);
#endif

      /* io_req.function identifies function which should be called */
      switch(io_req.function)
      {
      /* Request to open a file */
      case MEMFS_DELEGATED_OPEN:
      case MEMFS_OPEN:
          {
          int pack_size;
          int reply_size;
          MPI_Status reply_status;
          char *hint;
          int flag, position, min_size;
          int remote_fd_sys[1];
          int remote_error[1];
          int64_t blocksize = 0;
          int filename_length = 0;
          char *filename;
          int destination;

#ifdef DEBUG_THREADS
          fprintf(stderr, "server %d received an open request for file %s\n", comm_rank, io_req.fd->filename);
#endif


          /* Handling of a file open request */

          if(io_req.fd->info != MPI_INFO_NULL) 
          {
              hint = (char *)malloc(MPI_MAX_INFO_VAL+1 * sizeof(char));
              LOCK_MPI();
              MPI_Info_get(io_req.fd->info, "blocksize", MPI_MAX_INFO_VAL, hint, &flag);
              UNLOCK_MPI();
              if(flag) {
                  blocksize = atoi(hint);
              }
              free(hint);
          }

          /* either this is the master server or more than 1 server does exist */
          assert(i_am_master || memfs_num_servers > 1);

/*          if(i_am_master || memfs_num_servers == 1) */
          if(1) 
          {
              io_req.fd->fd_sys = open_file(io_req.fd->filename, io_req.fd->access_mode,
                                            &io_req.error, blocksize, -1);
          } else {
              io_req.fd->fd_sys = -1;
          }

          /* File data distribution is only necessary if more than 1 server is involved */
          if(memfs_num_servers > 1) {
              position = 0;

              LOCK_MPI();
              min_size = 0;
              filename_length = strlen(io_req.fd->filename) + 2; 
              MPI_Pack_size(filename_length, MPI_CHAR, MPI_COMM_MEMFS_WORLD, &pack_size);
              min_size += pack_size;
              MPI_Pack_size(3, MPI_INT, MPI_COMM_MEMFS_WORLD, &pack_size);
              min_size += pack_size;
              MPI_Pack_size(1, ADIO_OFFSET, MPI_COMM_MEMFS_WORLD, &pack_size);
              min_size += pack_size;
              UNLOCK_MPI();
              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
 
              LOCK_MPI();

              filename = (char *)malloc(filename_length * sizeof(char));
              strcpy(filename, io_req.fd->filename);

              MPI_Pack(&filename_length, 1, MPI_INT, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              MPI_Pack(filename, filename_length, MPI_CHAR, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              MPI_Pack(&io_req.fd->access_mode, 1, MPI_INT, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              MPI_Pack(&blocksize, 1, ADIO_OFFSET, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              MPI_Pack(&io_req.fd->fd_sys, 1, MPI_INT, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

              /* If this server is not the master -> send data to master */
              /* Else send data to "clients" */

              if(!i_am_master) 
              {
                  /* this is not the master server, delegate request */
                  ;
/*
                  #ifdef DEBUG_THREADS
                  fprintf(stderr, "[m%d]: Sending open data to master server %d\n", comm_rank, master_server);
                  #endif
                  dest = pario_map_r2w(master_server);
                  ptMPI_Send(send_buf, position, MPI_PACKED,
                             dest, MEMFS_DELEGATED_OPEN,
                             TUNNELFS_COMM_WORLD);
*/

              } else {
                  /* this is the master server, handle request */
                  /* send open data to all memfs servers */
                  for(destination = 0; destination < comm_size; destination++) {
                      if(destination != comm_rank) 
                      {
                        // dest = pario_map_r2w(destination);
                        #ifdef DEBUG_THREADS
                          fprintf(stderr, "[m%d]: Sending open data to rank %d\n", comm_rank, destination);
                        #endif
                          ptMPI_Send(send_buf, position, MPI_PACKED,
                                     destination, MEMFS_OPEN,
                                     MPI_COMM_MEMFS_WORLD);
                      }
                  }
              }
              free(filename);
              /* Wait for results of remote operations now */
              /* distinguish between master and client servers */

              if(!i_am_master) 
              {
                  /* The request was delegated to another server */
                  /* this is not the master server, receive result from master server */
;
/*
                  get_reply(&reply_status, &reply_size, MEMFS_REPLY);
                  position = 0;
                  LOCK_MPI();
                  MPI_Unpack(recv_buf, reply_size, &position, &remote_fd_sys, 1, MPI_INT,
                             TUNNELFS_COMM_WORLD);
                  MPI_Unpack(recv_buf, reply_size, &position, &remote_error, 1, MPI_INT,
                             TUNNELFS_COMM_WORLD);
                  UNLOCK_MPI();
#ifdef DEBUG_THREADS
                  fprintf(stderr, "[m%d]: results received from master server: fd_sys: %d, error: %d\n", 
                          comm_rank, remote_fd_sys[0], remote_error[0]);
#endif
                  io_req.fd->fd_sys = remote_fd_sys[0];
                  io_req.error = remote_error[0];
*/
              } else { 
                  /* this is the master server, receive results */
                  for(destination = 0; destination < comm_size-1; destination++) 
                  {
                      get_reply(&reply_status, &reply_size, MEMFS_REPLY);
                      position = 0;
                      LOCK_MPI();
                      MPI_Unpack(recv_buf, reply_size, &position, &(remote_fd_sys[0]), 1, MPI_INT,
                                 MPI_COMM_MEMFS_WORLD);
                      MPI_Unpack(recv_buf, reply_size, &position, &(remote_error[0]), 1, MPI_INT,
                                 MPI_COMM_MEMFS_WORLD);
                      UNLOCK_MPI();
#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: remote fd_sys value: %d, remote error: %d\n", 
                              comm_rank, remote_fd_sys[0], remote_error[0]);
#endif
                   }
              }

              if(io_req.function == MEMFS_DELEGATED_OPEN) {
                  /* send results to io_req.param which is the original server */
#ifdef DEBUG_THREADS                  
                  fprintf(stderr, "[m%d]: sending results of DELEGATED_OPEN to server %d\n",
                          comm_rank, io_req.param);
#endif                          
                  int fd_sys;

                  /* Pack values of reply message */
                  LOCK_MPI();
                  MPI_Pack_size(2, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
                  UNLOCK_MPI();
                  tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);

                  LOCK_MPI();
                  position = 0;
                  fd_sys = io_req.fd->fd_sys;
                  MPI_Pack(&fd_sys, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  MPI_Pack(&io_req.error, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  UNLOCK_MPI();

                  /* Send MEMFS_REPLY message to source of request */
                  // dest = pario_map_r2w(io_req.param);
                  ptMPI_Send(send_buf, position, MPI_PACKED,
                             io_req.param, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);
              
              }
              /* End of Communication with remote MEMFS Servers */

          } /* if memfs_num_servers > 1 */
          break;
          }
       case MEMFS_CLOSE:
          {
          int reply_size;
          MPI_Status reply_status;
          int position, min_size;
          int remote_error[comm_size];
          int destination;

#ifdef DEBUG_MEMFS
          fprintf(stderr, "[m%d]: MEMFS_CLOSE: Closing file\n", comm_rank);
#endif

          if(memfs_num_servers > 1) {

          /* Initializing Communication with other MEMFS Servers */
          position = 0;
          LOCK_MPI();
          MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
          UNLOCK_MPI();
          tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);

          LOCK_MPI();
          MPI_Pack(&io_req.fd->fd_sys, 1, MPI_INT, send_buf, send_buf_size,
                   &position, MPI_COMM_MEMFS_WORLD);
          UNLOCK_MPI();

          /* TODO: ONLY IDENTIFY SERVERS WHICH HAVE SOME POTION OF THE FILE */
          for(destination = 0; destination < comm_size; destination++) {
              if(destination != comm_rank) {
#ifdef DEBUG_THREADS
                  fprintf(stderr, "[m%d]:, MEMFS_CLOSE: Sending data to rank %d\n", comm_rank, destination);
#endif
                  // dest = pario_map_r2w(destination);
                  ptMPI_Send(send_buf, position, MPI_PACKED,
                             destination, MEMFS_CLOSE,
                             MPI_COMM_MEMFS_WORLD);
              }
          }

          } /* if memfs_num_servers > 1 */

          /* Communication with other MEMFS Servers now initialized */

          /*
           * Request submitted.
           * local operations may be done now
           * Receive Results of remote operations after that
           */
          close_file(io_req.fd->fd_sys, &io_req.error);

          if(memfs_num_servers > 1) 
          {
          /* Wait for results of remote operations now */

          for(destination = 0; destination < comm_size-1; destination++) {
              get_reply(&reply_status, &reply_size, MEMFS_REPLY);
              position = 0;
              LOCK_MPI();
              MPI_Unpack(recv_buf, reply_size, &position, 
                         &(remote_error[reply_status.MPI_SOURCE]), 
                         1, MPI_INT, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();
#ifdef DEBUG_THREADS
              fprintf(stderr, "[m%d]: MEMFS_CLOSE: Remote error code was: %d\n", 
                      comm_rank, remote_error[reply_status.MPI_SOURCE]);
#endif
          }
          } /* if memfs_num_servers > 1 */
          /* End of Communication with remote MEMFS Servers */

          break;
          }
       case MEMFS_READCONT:
          {
          int position, original_pos, min_size, packsize, i, len;
          int num_blocks, datatype_size, block_count, charsize, start_block, start_server;
          ADIO_Offset offset_at_server[comm_size];
          ADIO_Offset increment[comm_size];
          int blocks_per_server[comm_size];
          int count;
          int destination;
          int reply_size;
          int remote_servers = 0;
          int remote_blocks = 0;
          int local_blocks = 0;
          int source;
          int block_offset;
          ADIO_Offset off;
          MPI_Status reply_status;
          ADIO_Offset global_end_at;
          ADIO_Offset local_end_at;
          int last_server;
          int last_block;

#ifdef DEBUG_THREADS
          fprintf(stderr, "server %d received a read request for file %d\n", comm_rank, io_req.fd->fd_sys);
#endif

          #ifdef MEMFS_TIME
          totaltime1 = gettime();
          #endif

          if(memfs_num_servers > 1) {

          /* compute blocks, blocksize, etc */

              position = 0;
              count = io_req.param;

              /* if MEMFS_BLOCKSIZE > MEMFS_MAX_MSG_SIZE special handling is needed */
              /* for now just assert that this is not the case! */
              //assert(MEMFS_BLOCKSIZE <= MEMFS_MAX_MSG_SIZE);

              /* compute number of io blocks */
              LOCK_MPI();
              MPI_Type_size(io_req.datatype, &datatype_size);
              MPI_Type_size(MPI_CHAR, &charsize);
              len = datatype_size * count;
              UNLOCK_MPI();
              num_blocks = (int) (len / MEMFS_BLOCKSIZE);
              if(len % MEMFS_BLOCKSIZE)
                  num_blocks++;
              if(((len % MEMFS_BLOCKSIZE) + (io_req.offset % MEMFS_BLOCKSIZE)) > MEMFS_BLOCKSIZE)
                  num_blocks++;


              start_block = io_req.offset / MEMFS_BLOCKSIZE;
              start_server = start_block % memfs_num_servers;

              /* compute number of blocks for each server */
              for(i = 0; i < memfs_num_servers; i++) {
                  blocks_per_server[i] = 0;
              }

              for(i = start_server; i < start_server + num_blocks; i++) {
                  blocks_per_server[i % memfs_num_servers] += 1;
                  if(i % memfs_num_servers != comm_rank) remote_blocks++;
                  else local_blocks++;
              }

              /* block_offset is the offset in the first involved data block */
              block_offset = (int) (MEMFS_BLOCKSIZE - (io_req.offset % MEMFS_BLOCKSIZE));

#ifdef DEBUG_THREADS
              fprintf(stderr, "[m%d]: MEMFS_READCONT: start_server: %d, start_block: %d\n",
                      comm_rank, start_server, start_block);
              fprintf(stderr, "[m%d]: MEMFS_READCONT: offset: %lld, bytecount: %d\n", 
                      comm_rank, io_req.offset, len);
              fprintf(stderr, "[m%d]: MEMFS_READCONT: numblocks: %d, local_blocks: %d, remote_blocks: %d, block_offset: %d\n",
                      comm_rank, num_blocks, local_blocks, remote_blocks, block_offset);
#endif

          /* LOCKING MECHANISM */

          /* Check if exclusive Flag is set.
           * If the flag is set calculate the blocks that
           * need to be locked. Call set_lock() to lock the blocks
           * before reading the blocks
           */   
          int exclusive=0,i, carry, carry2, first_block;
          int lock_error=0, num_data_blocks;
          int64_t block_size;
          int ADIO_Offset_size;
          int *blocks;
          char* hint;
          int flag;

          /* Check if exclusive flag is set to 0 (not locked) 
           * or 1 (file locked)
           */

              if(io_req.fd->info != MPI_INFO_NULL)
              {
                hint = (char *)malloc(MPI_MAX_INFO_VAL+1 * sizeof(char));
                LOCK_MPI();
                MPI_Info_get(io_req.fd->info, "exclusive", MPI_MAX_INFO_VAL, hint, &flag);
                UNLOCK_MPI();
                if(flag) {
                   exclusive = atoi(hint);
                }
                free(hint);
              }


#ifdef DEBUG_LOCKS
          fprintf(stderr, "[m%d]: 1. Exclusive set to %d \n", comm_rank,exclusive);
#endif

          /* If the file is locked (exclusive==1) send a request to lock
           * the file blocks that need to be read or written
               */
          if(exclusive == 1){
                block_size = memfs_get_blocksize((io_req.fd)->fd_sys);
            num_data_blocks = (len / block_size);

            if( (len % block_size) > 0 )
            num_data_blocks ++;
        
            carry  = len % block_size;
                carry2 = (io_req.offset) % block_size;

                if(carry + carry2 > block_size) {
                num_data_blocks++;
                }

            MPI_Type_size(ADIO_OFFSET, &ADIO_Offset_size);
            blocks = (int *) malloc(num_data_blocks * sizeof(int));

            // Set all block fields to the block number that needs to be locked
            first_block = io_req.offset / block_size;
            for(i = 0; i < num_data_blocks; i++){
            blocks[i] = first_block + i;
            }

                #ifdef MEMFS_TIME
                t1 = gettime();
                #endif

                set_lock((io_req.fd)->fd_sys, blocks, num_data_blocks, memfs_num_servers, &lock_error);

                #ifdef MEMFS_TIME
                t2 = gettime();
                settime(READ_SETLOCK, t2 - t1);
                #endif

#ifdef DEBUG_LOCKS
                fprintf(stderr, "[m%d]: set_lock() fh %d, blocks %d, num_data_blocks %d \n", 
                comm_rank,(io_req.fd)->fd_sys, *blocks, num_data_blocks);
                fprintf(stderr, "[m%d]: set_lock() successfull \n", comm_rank);
#endif
              }



              for(i = 0; i < memfs_num_servers; i++) {
                  /* remote_servers identifies how many servers are involved
                   * in this read request
                   */
                  if(blocks_per_server[i] > 0 && comm_rank != i)
                       remote_servers++;

                  /* compute local offset at each server */
                  offset_at_server[i] = (start_block / memfs_num_servers) * MEMFS_BLOCKSIZE;
                  if (i == start_server) {
                      /* this server has to regard the global file offset */
                      offset_at_server[i] += io_req.offset % MEMFS_BLOCKSIZE;
                  }
                  /* start at next block */
                  if(i < start_server) {
                      offset_at_server[i] += MEMFS_BLOCKSIZE;
                  }
                  increment[i] = 0;

              }
              
#ifdef DEBUG_THREADS 
              for(i = 0; i < memfs_num_servers; i++) {
                  fprintf(stderr, "[m%d]: MEMFS_READCONT: offset_at_server[%d]: %lld\n", comm_rank, i, offset_at_server[i]);              
              }
#endif


          /* send setup information to other servers */
              if(remote_servers > 0) {

                  #ifdef MEMFS_TIME
                  t1 = gettime();
                  #endif

                  position = 0;
                  LOCK_MPI();
                  /* Pack and Send Setup message of Write request */
                  MPI_Pack_size(5, MPI_INT, MPI_COMM_MEMFS_WORLD, &packsize);
                  min_size = packsize;
                  MPI_Pack_size(2, ADIO_OFFSET, MPI_COMM_MEMFS_WORLD, &packsize);
                  min_size += packsize;

                  tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
                  global_end_at = len + io_req.offset;

                  ++main_msg_id;
                  MPI_Pack(&main_msg_id, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  MPI_Pack(&(io_req.fd)->fd_sys, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  /* All IO Data is sent as type MPI_BYTE */
                  MPI_Pack(&charsize, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  MPI_Pack(&io_req.file_ptr_type, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  MPI_Pack(&global_end_at, 1, ADIO_OFFSET, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  UNLOCK_MPI();
                  for(destination = 0; destination < comm_size; destination++) {
                      if(destination != comm_rank && blocks_per_server[destination] > 0) {
#ifdef DEBUG_THREADS
                          fprintf(stderr, "[m%d]: MEMFS_READCONT: Sending read setup data to rank %d, main_msg_id: %d, offset_at_server: %lld\n",
                                  comm_rank, destination, main_msg_id, offset_at_server[destination]);
#endif
                          LOCK_MPI();
                          original_pos = position;
                          MPI_Pack(&(blocks_per_server[destination]), 1, MPI_INT, send_buf, send_buf_size,
                                   &position, MPI_COMM_MEMFS_WORLD);
                          MPI_Pack(&offset_at_server[destination], 1, ADIO_OFFSET, send_buf, send_buf_size,
                                   &position, MPI_COMM_MEMFS_WORLD);

                          UNLOCK_MPI();
                          // dest = pario_map_r2w(destination);
                          ptMPI_Send(send_buf, position, MPI_PACKED,
                                     destination, MEMFS_READCONT,
                                     MPI_COMM_MEMFS_WORLD);
#ifdef DEBUG_THREADS
fprintf(stderr, "[m%d]: sent to %d (global %d)\n", comm_rank, destination, dest);
#endif
                          position = original_pos;
                      }
                  }
                  #ifdef MEMFS_TIME
                  t2 = gettime();
                  settime(READ_SETUP, t2 - t1);
                  #endif

                  /* read data from other servers and from local server */                  
                  for(i = 0; i < remote_blocks; i++) {
                      #ifdef MEMFS_TIME
                      t1 = gettime();
                      #endif
#ifdef DEBUG_THREADS
fprintf(stderr, "[m%d]: receiving reply\n", comm_rank);
#endif
                      get_reply(&reply_status, &reply_size, MEMFS_REPLY_IODATA);
                      /* offset is dependent of reply source */
                      // source = pario_map_w2r(reply_status.MPI_SOURCE);
                      source = reply_status.MPI_SOURCE;
                      assert(reply_status.MPI_TAG == MEMFS_REPLY_IODATA);
                      /** 
                       * compute buffer offset for read 
                       * 1. line = Compute read buffer offset out of server_id (source), number of servers, offset and MEMFS_BLOCKSIZE
                       * 2. line = offset for consequent replies of the same server is incremented by MEMFS_BLOCKSIZE * memfs_num_servers
                       * 3. line = Restrict values to be >= 0 
                       */

                      off = ((source + memfs_num_servers - start_server) % memfs_num_servers) * MEMFS_BLOCKSIZE - (io_req.offset % MEMFS_BLOCKSIZE);
                      off += (increment[source] / MEMFS_BLOCKSIZE) * memfs_num_servers * MEMFS_BLOCKSIZE;
                      if(off < 0) off = 0;

#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: MEMFS_READCONT REMOTE BLOCK: offset is calculated as: %lld, reply_size: %d, remote_blocks: %d\n", 
                              comm_rank, off, reply_size, remote_blocks);
#endif
                      /* copy receive buffer into io request buffer */
                      memcpy(io_req.buffer + off, recv_buf, reply_size);
#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: MEMFS_READCONT: memcpy complete\n",
                              comm_rank);
#endif

                       increment[source] += MEMFS_BLOCKSIZE;

                       #ifdef MEMFS_TIME
                       t2 = gettime();
                       settime(REMOTE_READ, t2 - t1);
                       #endif
                  }

#ifdef DEBUG_THREADS
                  fprintf(stderr, "[m%d]: MEMFS_READCONT: finished receive of remote blocks\n", comm_rank);
#endif

                  local_end_at = (global_end_at / memfs_num_servers);
                  local_end_at += MEMFS_BLOCKSIZE - (local_end_at % MEMFS_BLOCKSIZE);

                  last_block = global_end_at / MEMFS_BLOCKSIZE;
                  if(global_end_at % MEMFS_BLOCKSIZE > 0) {
                      last_block++;
                  }
                  last_server = (last_block - 1) % memfs_num_servers;
  
                  if(last_server == comm_rank && (global_end_at % MEMFS_BLOCKSIZE) > 0) {
                      /* this is the server which holds the last block of the request */
                      local_end_at -= MEMFS_BLOCKSIZE - (global_end_at % MEMFS_BLOCKSIZE);
                  }
                  if(comm_rank > last_server) {
                      local_end_at -= MEMFS_BLOCKSIZE;
                  }

#ifdef DEBUG_THREADS
                  fprintf(stderr, "[m%d]: MEMFS_READCONT: local_end_at: %lld\n", comm_rank, local_end_at);
#endif

                  /* now do the local read */
                  source = comm_rank;

                  #ifdef MEMFS_TIME
                  t1 = gettime();
                  #endif

                  for(i = 0; i < local_blocks; i++) {
                      if(i == 0) {
                          /* first part can be shorter */
                          block_count = (MEMFS_BLOCKSIZE - (offset_at_server[comm_rank] % MEMFS_BLOCKSIZE)) / datatype_size;
                          if(block_count > ((offset_at_server[comm_rank] + local_end_at) / datatype_size)) {
                              block_count = (offset_at_server[comm_rank] + local_end_at) / datatype_size;
                          }
                      } else {
                          block_count = MEMFS_BLOCKSIZE / datatype_size;
                      }
                      if (i == local_blocks - 1) {
                          /* last part can be shorter */
                          block_count = (local_end_at - (offset_at_server[comm_rank] + increment[source])) / datatype_size;
                      }
#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: MEMFS_READCONT: source: %d, start_server: %d, io_req.offset: %lld\n", 
                              comm_rank, source, start_server, io_req.offset);
#endif
                      off = ((source + memfs_num_servers - start_server) % memfs_num_servers) * MEMFS_BLOCKSIZE - (io_req.offset % MEMFS_BLOCKSIZE);
                      off += (increment[source] / MEMFS_BLOCKSIZE) * memfs_num_servers * MEMFS_BLOCKSIZE;
                      if(off < 0) off = 0;

#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: MEMFS_READCONT LOCAL BLOCK:", comm_rank);
                      fprintf(stderr, "writing at off %lld, reading from offset_at_server: %lld, block_count: %d\n", 
                              off, offset_at_server[source] + increment[source], block_count);
#endif
                      /* Local read for this block is performed now */
                      read_file_contig(io_req.fd, io_req.buffer + off, block_count,
                                       io_req.datatype, io_req.file_ptr_type,
                                       offset_at_server[source] + increment[source], io_req.status, &io_req.error);

#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: MEMFS_READCONT: writing at off %lld finished\n",
                              comm_rank, off);
#endif


                      if(offset_at_server[source] % MEMFS_BLOCKSIZE != 0)
                          offset_at_server[source] -= offset_at_server[source] % MEMFS_BLOCKSIZE;

                       increment[source] += MEMFS_BLOCKSIZE;
                  }
                  #ifdef MEMFS_TIME
                  t2 = gettime();
                  settime(LOCAL_READ, t2 - t1);
                  #endif
 

              } else {
                  /* more than 1 memfs server exists but only one is needed for this request */
#ifdef DEBUG_THREADS
                  fprintf(stderr, "memfs_main: READ LOCAL\n");
#endif

                  #ifdef MEMFS_TIME
                  t1 = gettime();
                  #endif

                  read_file_contig(io_req.fd, io_req.buffer, io_req.param,
                                   io_req.datatype, io_req.file_ptr_type,
                                   offset_at_server[comm_rank], io_req.status, &io_req.error);

                  #ifdef MEMFS_TIME
                  t2 = gettime();
                  settime(LOCAL_READ, t2 - t1);
                  #endif
              }

          if(exclusive == 1) {
                #ifdef MEMFS_TIME
                t1 = gettime();
                #endif

                remove_lock((io_req.fd)->fd_sys, blocks, num_data_blocks, memfs_num_servers, &lock_error);
                free(blocks);

                #ifdef MEMFS_TIME
                t2 = gettime();
                settime(READ_REMOVELOCK, t2-t1);
                #endif

#ifdef DEBUG_LOCKS
                fprintf(stderr, "[m%d]: remove_lock fh %d, blocks %d, num_data_blocks %d \n",
                                comm_rank,(io_req.fd)->fd_sys, *blocks, num_data_blocks);
                fprintf(stderr, "[m%d]: remove_lock successful \n",comm_rank);
        //free(blocks);
#endif
          }

          } else {
              /* only 1 memfs server exists */
              /* io_req.param is the count of items to read */
#ifdef DEBUG_THREADS
              fprintf(stderr, "memfs_main: READ LOCAL\n");
#endif

              #ifdef MEMFS_TIME
              t1 = gettime();
              #endif

              read_file_contig(io_req.fd, io_req.buffer, io_req.param, 
                               io_req.datatype, io_req.file_ptr_type, 
                               io_req.offset, io_req.status, &io_req.error);

              #ifdef MEMFS_TIME
              t2 = gettime();
              settime(LOCAL_READ, t2 - t1);
              #endif

          }

          #ifdef MEMFS_TIME
          totaltime2 = gettime();
          settime(READ_MAIN, totaltime2 - totaltime1);
          #endif
      break;
          }
       case MEMFS_READSTRIDED:
          {
          read_file_strided(io_req.fd, io_req.buffer, io_req.param,
                            io_req.datatype, io_req.file_ptr_type, 
                            io_req.offset, io_req.status, &io_req.error);        
          }
       case MEMFS_WRITECONT:
          {
          int position, original_pos, min_size, packsize, i, len;
          int num_blocks, datatype_size, block_count, charsize, start_block, start_server;
          ADIO_Offset offset_at_server[comm_size];
          int blocks_per_server[comm_size];
          ADIO_Offset buf_offset = 0;
          int count;
          int destination;
          int reply_size;
          int remote_servers = 0;
          int remote_error[1];
          MPI_Status reply_status;

#ifdef DEBUG_THREADS
          fprintf(stderr, "server %d received a write request for file %d\n", comm_rank, io_req.fd->fd_sys);
#endif


#ifdef DEBUG_THREADS
          fprintf(stderr, "[m%d]: Writecont request, num_servers: %d\n", 
                  comm_rank, memfs_num_servers);
#endif

          #ifdef MEMFS_TIME
          totaltime1 = gettime();
          #endif

          count = io_req.param;


          if(memfs_num_servers > 1) {
              position = 0;

              /* if MEMFS_BLOCKSIZE > MEMFS_MAX_MSG_SIZE special handling is needed */
              /* for now just assert that this is not the case! */
              //assert(MEMFS_BLOCKSIZE <= MEMFS_MAX_MSG_SIZE); 

              /* compute number of io blocks */
              LOCK_MPI();
              MPI_Type_size(io_req.datatype, &datatype_size);
              MPI_Type_size(MPI_CHAR, &charsize);
              len = datatype_size * count;
              UNLOCK_MPI();
              num_blocks = (int) (len / MEMFS_BLOCKSIZE);
              if(len % MEMFS_BLOCKSIZE)
                  num_blocks++;
              if(((len % MEMFS_BLOCKSIZE) + (io_req.offset % MEMFS_BLOCKSIZE)) > MEMFS_BLOCKSIZE)
                  num_blocks++;

#ifdef DEBUG_DATADIST
              fprintf(stderr, "[m%d]: MEMFS_WRITECONT: datatype_size: %d, count: %d, len: %d\n", 
                      comm_rank, datatype_size, count, len);
              fprintf(stderr, "[m%d]: MEMFS_WRITECONT: num_blocks used: %d\n", 
                      comm_rank, num_blocks);
#endif

              start_block = io_req.offset / MEMFS_BLOCKSIZE;
              start_server = start_block % memfs_num_servers;          

              assert(start_block >= 0);
              assert(start_server >= 0);

              /* compute number of blocks for each server */
              for(i = 0; i < memfs_num_servers; i++) {
                  blocks_per_server[i] = 0;
              }

              for(i = start_server; i < start_server + num_blocks; i++) {
                  blocks_per_server[i % memfs_num_servers] += 1;
              }


              for(i = 0; i < memfs_num_servers; i++) {
                  /* remote_servers identifies how many remote servers are involved 
                   * in the storage task of this write 
                   */
                  if(blocks_per_server[i] > 0 && comm_rank != i)
                       remote_servers++;

                  /* compute local offset at each server */
                  offset_at_server[i] = (start_block / memfs_num_servers) * MEMFS_BLOCKSIZE;
                  if (i == start_server) {
                      /* this server has to regard the global file offset */
                      offset_at_server[i] += io_req.offset % MEMFS_BLOCKSIZE;
                  }
                  if(i < start_server) {
                      /* start at next block */
                      offset_at_server[i] += MEMFS_BLOCKSIZE;
                  }
                  if(offset_at_server[i] < 0) {
                      fprintf(stderr, "offset_at_server[%d]: %lld\n", i, offset_at_server[i]);
                      fprintf(stderr, "start_server: %d, start_block: %d, memfs_num_servers: %d, MEMFS_BLOCKSIZE: %d\n", 
                              start_server, start_block, memfs_num_servers, MEMFS_BLOCKSIZE);
                  }

                  if(offset_at_server[i] < 0) 
                      break;
           
              }

#ifdef DEBUG_DATADIST
              fprintf(stderr, "[m%d]: MEMFS_WRITECONT: startblock: %d, start_server: %d\n", comm_rank, start_block, start_server);
              for(i = 0; i < memfs_num_servers; i++) {
                  fprintf(stderr, "[m%d]: MEMFS_WRITECONT: blocks_per_server[%d]: %d\n", comm_rank, i, blocks_per_server[i]);
              }
#endif

              /* Check if exclusive Flag is set.
               * If the flag is set calculate the blocks that
               * need to be locked. Call set_lock() to lock the blocks
               * before reading the blocks
               */
              int exclusive=0,i, carry, carry2, first_block;
              int lock_error=0, num_data_blocks;
              int64_t block_size;
              int ADIO_Offset_size;
              int *blocks;
              char* hint;
              int flag;

              /* Check if exclusive flag is set to 0 (not locked)
               * or 1 (file locked)
               */

              if(io_req.fd->info != MPI_INFO_NULL)
              {
                hint = (char *)malloc(MPI_MAX_INFO_VAL+1 * sizeof(char));
                LOCK_MPI();
                MPI_Info_get(io_req.fd->info, "exclusive", MPI_MAX_INFO_VAL, hint, &flag);
                UNLOCK_MPI();
                if(flag) {
                   exclusive = atoi(hint);
                }
                free(hint);
              }

#ifdef DEBUG_LOCKS
              fprintf(stderr, "[m%d]: 1. Exclusive set to %d \n", comm_rank,exclusive);
#endif

              /* If the file is locked (exclusive==1) send a request to lock
               * the file blocks that need to be read or written
               */
              if(exclusive == 1){
                block_size = memfs_get_blocksize((io_req.fd)->fd_sys);
                num_data_blocks = (len / block_size);

                if( (len % block_size) > 0 )
                        num_data_blocks ++;

                carry  = len % block_size;
                carry2 = (io_req.offset) % block_size;

                if(carry + carry2 > block_size) {
                        num_data_blocks++;
                }

                MPI_Type_size(ADIO_OFFSET, &ADIO_Offset_size);
                blocks = (int *) malloc(num_data_blocks * sizeof(int));

                // Set all block fields to the block number that needs to be locked
                first_block = io_req.offset / block_size;
                for(i = 0; i < num_data_blocks; i++){
                        blocks[i] = first_block + i;
                }

        #ifdef MEMFS_TIME
        t1 = gettime();
        #endif
                set_lock((io_req.fd)->fd_sys, blocks, num_data_blocks, memfs_num_servers, &lock_error);
                #ifdef MEMFS_TIME
                t2 = gettime();
                settime(WRITE_SETLOCK,t2-t1);
                #endif

#ifdef DEBUG_LOCKS
                fprintf(stderr, "[m%d]: set_lock() fh %d, blocks %d, num_data_blocks %d \n",
                                comm_rank,(io_req.fd)->fd_sys, *blocks, num_data_blocks);
                fprintf(stderr, "[m%d]: set_lock() successfull \n", comm_rank);
#endif
              }

              /* setup messages are only neccessary if remote servers are participating */
              if(remote_servers > 0) {
                  #ifdef MEMFS_TIME
                  t1 = gettime();
                  #endif
                  position = 0;
                  LOCK_MPI();
                  /* Pack and Send Setup message of Write request */
                  MPI_Pack_size(5, MPI_INT, MPI_COMM_MEMFS_WORLD, &packsize);
                  min_size = packsize;
                  MPI_Pack_size(1, ADIO_OFFSET, MPI_COMM_MEMFS_WORLD, &packsize);
                  min_size += packsize;

                  tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
    
                  main_msg_id++;
                  MPI_Pack(&main_msg_id, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  MPI_Pack(&(io_req.fd)->fd_sys, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  /* All IO Data is sent as type MPI-CHAR */
                  MPI_Pack(&charsize, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  MPI_Pack(&io_req.file_ptr_type, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  UNLOCK_MPI();

                  /* TODO: ONLY IDENTIFY SERVERS WHICH HAVE SOME POTION OF THE FILE */
                  for(destination = 0; destination < comm_size; destination++) {
                      if(destination != comm_rank && blocks_per_server[destination] > 0) {
#ifdef DEBUG_THREADS
                          fprintf(stderr, "[m%d]: MEMFS_WRITECONT: main_msg_id: %d, Sending write setup data to: %d\n", 
                                  comm_rank, main_msg_id, destination);
#endif
                          LOCK_MPI();
                          original_pos = position;
                          MPI_Pack(&(blocks_per_server[destination]), 1, MPI_INT, send_buf, send_buf_size,
                                   &position, MPI_COMM_MEMFS_WORLD);
                          MPI_Pack(&(offset_at_server[destination]), 1, ADIO_OFFSET, send_buf, send_buf_size,
                                   &position, MPI_COMM_MEMFS_WORLD);
                          UNLOCK_MPI();
                          // dest = pario_map_r2w(destination);
                          ptMPI_Send(send_buf, position, MPI_PACKED,
                                     destination, MEMFS_WRITECONT,
                                     MPI_COMM_MEMFS_WORLD);
                          position = original_pos;
                      }
                  }

                  #ifdef MEMFS_TIME
                  t2 = gettime();
                  settime(WRITE_SETUP, t2 - t1);
                  #endif

                  /* Send parts of the buffer of Write request (the IO Data) */
    
                  /* sending io data */
                  for (i = 0; i < num_blocks; i++)
                  {
                      if(i == 0) {
                          /* first part can be shorter */
                          block_count = MEMFS_BLOCKSIZE - (io_req.offset % MEMFS_BLOCKSIZE);
                          if(block_count > len) {
                              block_count = len;
                          }
                      } else {
                          block_count = MEMFS_BLOCKSIZE;
                      }
                      if (i == num_blocks - 1) {
                          /* last part can be shorter */
                          block_count = len - buf_offset;
                      }

                      if(((start_server + i) % comm_size) != comm_rank) 
                      {
#ifdef DEBUG_THREADS
                          fprintf(stderr, "[m%d]: WRITECONT: sending data of block %d to proc %d, total blocks: %d\n", 
                                  comm_rank, i, (start_server + i) % comm_size, num_blocks);
#endif
                          #ifdef MEMFS_TIME
                          t1 = gettime();
                          #endif

                          /* dont send data to yourself, server! */
                          // dest = pario_map_r2w((start_server + i) % comm_size);
                          dest = (start_server + i) % comm_size;
                          ptMPI_Send(io_req.buffer + buf_offset, block_count, MPI_BYTE,
                                     dest, MEMFS_IODATA, MPI_COMM_MEMFS_WORLD);
                          #ifdef MEMFS_TIME
                          t2 = gettime();
                          settime(REMOTE_WRITE, t2 - t1);
                          #endif

                       } else {
#ifdef DEBUG_DATADIST
                           fprintf(stderr, "[m%d]: local write_file_contig block %d, total blocks: %d, buf_offset: %lld\n",
                                   comm_rank, i, num_blocks, buf_offset); 
                           fprintf(stderr, "[m%d]: offset_at_server: %lld, block_count: %d\n", 
                                   comm_rank, offset_at_server[comm_rank], block_count);
#endif
                           assert(offset_at_server[comm_rank] >= 0);

                           #ifdef MEMFS_TIME
                           t1 = gettime();
                           #endif

                           write_file_contig(io_req.fd, io_req.buffer + buf_offset, block_count,
                                             MPI_BYTE, io_req.file_ptr_type, offset_at_server[comm_rank], 
                                             io_req.status, &io_req.error);

                           #ifdef MEMFS_TIME
                           t2 = gettime();
                           settime(FILESYSTEM_WRITE, t2 - t1);
                           #endif

#ifdef DEBUG_DATADIST
                          fprintf(stderr, "[m%d]: local write_file_contig complete\n",
                                  comm_rank);
#endif

                           offset_at_server[comm_rank] += block_count; 
                      }
                      buf_offset += block_count;
                  }
#ifdef DEBUG_DATADIST
                  fprintf(stderr, "[m%d]: write_file_contig: ALL BLOCKS WRITTEN\n", comm_rank);
#endif

                  #ifdef MEMFS_TIME
                  t1 = gettime();
                  #endif



/*
                  for(destination = 0; destination < remote_servers; destination++) {
//#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: waiting for reply %d of %d, last received msg_id: %d\n", 
                              comm_rank, destination+1, remote_servers, msg_id);
//#endif
                      get_reply(&reply_status, &reply_size, MEMFS_REPLY_ERRORCODE);
                      position = 0;
                      LOCK_MPI();
                      assert(reply_status.MPI_TAG == MEMFS_REPLY_ERRORCODE);
                      MPI_Unpack(recv_buf, reply_size, &position, &msg_id,
                                 1, MPI_INT, TUNNELFS_COMM_WORLD);
                      MPI_Unpack(recv_buf, reply_size, &position, &(remote_error[0]), 
                                 1, MPI_INT, TUNNELFS_COMM_WORLD);
                      UNLOCK_MPI();
#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]: WRITECONT id: %d Remote error code was: %d\n", 
                              comm_rank, msg_id, remote_error[0]);
#endif
                      if(remote_error[0] != 0) {

                          // remote server returned an error
                          // set error code to failed
                              

                          io_req.error = MEMFS_IO_FAILURE;
                      }
                  } 

*/


                  #ifdef MEMFS_TIME
                  t2 = gettime();
                  settime(WRITE_GETERROR, t2 - t1);
                  #endif


              } /* if remote_servers > 1 */
              else {
                  /* more than 1 memfs server exists but only one is needed for this request */
                  if(offset_at_server[comm_rank] < 0) break;

                  #ifdef MEMFS_TIME
                  t1 = gettime();
                  #endif
   
                  write_file_contig(io_req.fd, io_req.buffer, count,
                                    io_req.datatype, io_req.file_ptr_type, offset_at_server[comm_rank],
                                    io_req.status, &io_req.error);

                  #ifdef MEMFS_TIME
                  t2 = gettime();
                  settime(FILESYSTEM_WRITE, t2 - t1);
                  #endif
              }

              if(exclusive == 1){
                #ifdef MEMFS_TIME
                t1 = gettime();
                #endif
                remove_lock((io_req.fd)->fd_sys, blocks, num_data_blocks, memfs_num_servers, &lock_error);
                free(blocks);
                #ifdef MEMFS_TIME
                t2 = gettime();
        settime(WRITE_REMOVELOCK,t2-t1);
                #endif

#ifdef DEBUG_LOCKS
                fprintf(stderr, "[m%d]: remove_lock fh %d, blocks %d, num_data_blocks %d \n",
                                comm_rank,(io_req.fd)->fd_sys, *blocks, num_data_blocks);
                fprintf(stderr, "[m%d]: remove_lock successful \n",comm_rank);
                //free(blocks);
#endif
              }
          } else {
              /* memfs_num_servers == 1 */
              if(io_req.offset < 0) break;

              #ifdef MEMFS_TIME
              t1 = gettime();
              #endif

              write_file_contig(io_req.fd, io_req.buffer, count,
                                io_req.datatype, io_req.file_ptr_type, io_req.offset,
                                io_req.status, &io_req.error);

              #ifdef MEMFS_TIME
              t2 = gettime();
              settime(FILESYSTEM_WRITE, t2 - t1);
              #endif
          }

          #ifdef MEMFS_TIME
          totaltime2 = gettime();
          settime(WRITE_MAIN, totaltime2 - totaltime1);
          #endif

          break;
          }
       case MEMFS_WRITESTRIDED:
          {
          fprintf(stderr, "Using MEMFS_WRITESTRIDED\n");
          write_file_strided(io_req.fd, io_req.buffer, io_req.param,
                             io_req.datatype, io_req.file_ptr_type, io_req.offset, 
                             io_req.status, &io_req.error);
          break;
          }
       case MEMFS_RESIZE:
          {
          int pack_size = 0;  
          int i;
          int reply_size;
          MPI_Status reply_status;

          int remote_error;
          int position, min_size, original_pos;
          int last_block, last_server;
          int destination;
          ADIO_Offset local_end_at[comm_size];


          if(memfs_num_servers > 1) {

              last_block = io_req.offset / MEMFS_BLOCKSIZE;
              if(io_req.offset % MEMFS_BLOCKSIZE > 0) {
                  last_block++;
              }
              last_server = (last_block - 1) % memfs_num_servers;

              for(i = 0; i < comm_size; i++) {
                  local_end_at[i] = (io_req.offset / memfs_num_servers);
                  local_end_at[i] += MEMFS_BLOCKSIZE - (local_end_at[i] % MEMFS_BLOCKSIZE);

                  if(last_server == i && (io_req.offset % MEMFS_BLOCKSIZE) > 0) {
                      /* this is the server which holds the last block of the request */
                      local_end_at[i] -= MEMFS_BLOCKSIZE - (io_req.offset % MEMFS_BLOCKSIZE);
                  }
                  if(i > last_server) {
                      local_end_at[i] -= MEMFS_BLOCKSIZE;
                  }
              }


              position = 0;

              LOCK_MPI();
              min_size = 0;
              MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &pack_size);
              min_size += pack_size;
              MPI_Pack_size(1, ADIO_OFFSET, MPI_COMM_MEMFS_WORLD, &pack_size);
              min_size += pack_size;
              UNLOCK_MPI();

              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
              LOCK_MPI();
              MPI_Pack(&io_req.fd->fd_sys, 1, MPI_INT, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

              for(destination = 0; destination < comm_size; destination++) {
                  if(destination != comm_rank) {
#ifdef DEBUG_THREADS
                      fprintf(stderr, "[m%d]:, MEMFS_RESIZE: Sending data to rank %d\n", comm_rank, destination);
#endif
                      LOCK_MPI();
                      original_pos = position;
                      MPI_Pack(&(local_end_at[destination]), 1, ADIO_OFFSET, send_buf, send_buf_size,
                               &position, MPI_COMM_MEMFS_WORLD);
                      UNLOCK_MPI();
                      // dest = pario_map_r2w(destination);
                      ptMPI_Send(send_buf, position, MPI_PACKED,
                                 destination, MEMFS_RESIZE,
                                 MPI_COMM_MEMFS_WORLD);
                      position = original_pos;
                  }
              }

              resize_file(io_req.fd, local_end_at[comm_rank], &io_req.error);

              /* Wait for results of remote operations now */

              for(destination = 0; destination < comm_size-1; destination++) {
                  get_reply(&reply_status, &reply_size, MEMFS_REPLY);
                  position = 0;
                  LOCK_MPI();
                  MPI_Unpack(recv_buf, reply_size, &position,
                             &remote_error,
                             1, MPI_INT, MPI_COMM_MEMFS_WORLD);
                  UNLOCK_MPI();
#ifdef DEBUG_THREADS
                  fprintf(stderr, "[m%d]: MEMFS_RESIZE: Remote error code was: %d\n",
                          comm_rank, remote_error);
#endif
                   if(remote_error != 0) {
                       /* server returned with an error */
                       /* set error code to failed */
                       io_req.error = MEMFS_IO_FAILURE;
                   }

              }
              /* End of Communication with remote MEMFS Servers */


          } else {
              /* memfs_num_servers == 1 */
              resize_file(io_req.fd, io_req.offset, &io_req.error);
          }

          break;
          }
       case MEMFS_DELETE:
          {
          int pack_size = 0;
          int reply_size;
          MPI_Status reply_status;

          int position, min_size;
          int remote_error[comm_size];
          int destination;
          int filename_length = 0;
          char *filename;

          filename_length = strlen(io_req.buffer) + 2;
          filename = (char *)malloc(filename_length * sizeof(char));
          strcpy(filename, io_req.buffer);

          assert(filename != NULL);

          if(memfs_num_servers > 1) {

          /* Initializing Communication with other MEMFS Servers */
          position = 0;

          LOCK_MPI();
          min_size = 0;
          MPI_Pack_size(filename_length, MPI_CHAR, MPI_COMM_MEMFS_WORLD, &pack_size);
          min_size += pack_size;
          MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &pack_size);
          min_size += pack_size;
          UNLOCK_MPI();
          tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);

          LOCK_MPI();
          MPI_Pack(&filename_length, 1, MPI_INT, send_buf, send_buf_size,
                   &position, MPI_COMM_MEMFS_WORLD);
          MPI_Pack(filename, filename_length, MPI_CHAR, send_buf, send_buf_size,
                   &position, MPI_COMM_MEMFS_WORLD);
          UNLOCK_MPI();

          /* TODO: ONLY IDENTIFY SERVERS WHICH HAVE SOME POTION OF THE FILE */
          for(destination = 0; destination < comm_size; destination++) {
              if(destination != comm_rank) {
#ifdef DEBUG_THREADS
                  fprintf(stderr, "[m%d]:, MEMFS_DELETE: Sending data to rank %d\n", comm_rank, destination);
#endif
                  // pario_map_r2w(destination);
                  ptMPI_Send(send_buf, position, MPI_PACKED,
                             destination, MEMFS_DELETE,
                             MPI_COMM_MEMFS_WORLD);
              }
          }

          } /* if memfs_num_servers > 1 */

          /* Communication with other MEMFS Servers now initialized */
          /*
           * Request submitted.
           * local operations may be done now
           * Receive Results of remote operations after that
          */

          delete_file(filename, &io_req.error);

          if(memfs_num_servers > 1)
          {
          /* Wait for results of remote operations now */

          for(destination = 0; destination < comm_size-1; destination++) {
              get_reply(&reply_status, &reply_size, MEMFS_REPLY);
              position = 0;
              LOCK_MPI();
              MPI_Unpack(recv_buf, reply_size, &position,
                         &(remote_error[reply_status.MPI_SOURCE]),
                         1, MPI_INT, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();
#ifdef DEBUG_THREADS
              fprintf(stderr, "[m%d]: MEMFS_DELETE: Remote error code was: %d\n",
                      comm_rank, remote_error[reply_status.MPI_SOURCE]);
#endif
          }
          } /* if memfs_num_servers > 1 */
          /* End of Communication with remote MEMFS Servers */


          break;
          }
       case MEMFS_FCNTL:
          {
          fcntl_file(io_req.fd, io_req.param, io_req.fcntl_struct, &io_req.error);
          break;
          }
       case MEMFS_SEEKIND:
          {
          ADIO_Offset offset;
          offset = seekind_file(io_req.fd, io_req.offset, io_req.param, &io_req.error);
          io_req.offset = offset;
          break;
          }
        case MEMFS_SETINFO:
          {
          setinfo_file(io_req.fd, (MPI_Info)io_req.extraParam, &io_req.error);
          break;
          }
#ifdef DEBUG_THREADS
      fprintf(stderr, "\n"); 
#endif
      }

      io_req.function = 0;
      pthread_cond_broadcast(&cond_finish_call);

      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&shutdown_mutex);
      if(memfs_shutdown) {
          pthread_mutex_unlock(&shutdown_mutex);
//          pthread_cond_broadcast(&message_for_memfs_service);
          break;
      }
      pthread_mutex_unlock(&shutdown_mutex);
    }

    free(recv_buf);
    free(send_buf);

#ifdef MEMFS_TIME
    printTimeMeasurement(comm_rank);
#endif

    pthread_mutex_unlock(&memfs_main_sync);

#ifdef DEBUG_THREADS
    fprintf(stderr, "memfs_main [%d] is exiting now\n", comm_rank);
#endif
    pthread_exit((void *) 0);
}

