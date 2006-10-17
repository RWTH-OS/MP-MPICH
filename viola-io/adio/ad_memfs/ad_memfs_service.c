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
* File:         ad_memfs_service.c                                        * 
* Description:  This is the "memfs service thread"                        *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/  

/*==========================================================================================================*/
/* INCLUDES */

#include "ad_memfs_service.h"
#include "ad_tunnelfs_common.h"
#include "ad_tunnelfs_msg.h"
#include "ad_memfs_lock.h"
#include "ad_memfs_files.h"
#include "pario_probe.h"
#include "pario_map.h"
/*#include "tunnelfs_srv.h"*/

#include "queue.h"

/*==========================================================================================================*/
/* DEFINES */

/*==========================================================================================================*/
/* TYPEDEFS */

/*==========================================================================================================*/
/* VARIABLES */

double t1, t2, t3, t4;
pthread_mutex_t memfs_service_sync = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_for_memfs_service = PTHREAD_COND_INITIALIZER;
MPI_Status memfs_service_status;
int memfs_service_status_is_empty = 1;

/*==========================================================================================================*/
/* STATIC FUNCTIONS */

/*==========================================================================================================*/
/* NON-STATIC FUNCTIONS */

/*
    function to handle requests from other memfs servers
*/
void *memfs_service(void *args) {
    int memfs_num_servers, flag, msg_size;
    MPI_Status msg_status;
    int recv_buf_size = 0;
    void *recv_buf = NULL;
    int send_buf_size = 0;
    void *send_buf = NULL;
    int msg_id = 1;
    int main_msg_id = 0;
    int min_size = 0;
    int comm_size, comm_rank;
    int i_am_master, master_server;
    int output;
    int dest;

    /* TODO: NEXT LINE DOES NOT WORK !? */
    memfs_num_servers = ((memfs_param_t *) args)->num_servers;

    /* Pthread param passing does not work right now! Use MPI_Comm_size as num_server param */
    MPI_Comm_size(MPI_COMM_MEMFS_WORLD, &memfs_num_servers);

    MPI_Comm_size(MPI_COMM_MEMFS_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_MEMFS_WORLD, &comm_rank);

    /* Select one master server */
    master_server = 0;
    if(comm_rank == master_server)
        i_am_master = 1;
    else
        i_am_master = 0;

    /* Create a queue to locking requests that failed at the first try */
    queue_t q;
    values_t v;
    int queue_size=0;
    q = queueCreate();
    //queuePrint(q);
    //queueIsEmpty(q);
 

#ifdef DEBUG_THREADS    
    fprintf(stderr, "Starting memfs_service Thread with %d Servers overall! rank: %d, size: %d\n", memfs_num_servers, comm_rank, comm_size);
#endif

    fprintf(stderr, "[s%d]: locking service mutex\n", comm_rank);
    pthread_mutex_lock(&memfs_service_sync);
 
    /* 
     * Wait for incoming requests 
     * Accept messages with tag range 0x3000 - 0x3fff
    */
    int output_done = 0;
    int print_tag = 1;
    int old_tag = 0;
    while(1) {
#ifndef USE_TUNNELFS_PROBE_THREAD
      LOCK_MPI();
      flag = 0;
/*      fprintf(stderr, "%d ", comm_rank); */

      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_MEMFS_WORLD, &flag, &msg_status);

#ifdef DEBUG_MEMFS
      if(flag && old_tag != msg_status.MPI_TAG) {
          print_tag = 1;
          old_tag = msg_status.MPI_TAG;
      }

      if(print_tag && flag) {
          fprintf(stderr, "[s%d]: Probe success. Tag: %x\n", comm_rank, msg_status.MPI_TAG);
          print_tag = 0;
      }
#endif


      if(flag && msg_status.MPI_TAG >= 0x3000 && msg_status.MPI_TAG <= 0x3fff) {
#ifdef DEBUG_THREADS
          fprintf(stderr, "\n[s%d]: handling request %d\n", comm_rank, msg_status.MPI_TAG);
#endif
          output_done = 0;
          print_tag = 1;
          /* Handle request */
          /* Retrieving size of next message */
          MPI_Get_count(&msg_status, MPI_PACKED, &msg_size);
          UNLOCK_MPI();

          /* checking bounds of receive buffer */
          tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, msg_size);

          ptMPI_Recv(recv_buf, msg_size, MPI_PACKED,
                     msg_status.MPI_SOURCE, msg_status.MPI_TAG,
                     MPI_COMM_MEMFS_WORLD, &msg_status);
/*          UNLOCK_MPI(); */

          output = 0;
#else

        /* initially locking mutex */
        fprintf(stderr, "[s%d]: clearing status\n", comm_rank);

        /* clearing the status field */
        memset(&memfs_service_status, 0, sizeof(MPI_Status));
        memfs_service_status_is_empty = 1;


#ifdef DEBUG_THREADS  
              fprintf(stderr, "memfs_service: [%d] waiting message_for_memfs_service in loop\n", comm_rank);
#endif
        /* wait for message signal */
        while ((memfs_service_status_is_empty) && !pario_shutdown)
            pthread_cond_wait(&message_for_memfs_service,
                              &memfs_service_sync);

        /* if thread is awakend for shutting down, exit! */
        if (pario_shutdown)
            break;
        
        pthread_mutex_lock(&shutdown_mutex);
        if(memfs_shutdown) {
            pthread_mutex_unlock(&shutdown_mutex);
            break;
        }
        pthread_mutex_unlock(&shutdown_mutex);

        if (memfs_service_status.MPI_TAG != MEMFS_IODATA)
        {
            LOCK_MPI();
            MPI_Get_count(&memfs_service_status, MPI_PACKED, &msg_size);

            tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, msg_size);

            MPI_Recv(recv_buf, msg_size, MPI_PACKED,
                     memfs_service_status.MPI_SOURCE,
                     memfs_service_status.MPI_TAG, MPI_COMM_MEMFS_WORLD,
                     &msg_status);
            UNLOCK_MPI();

            /**
             * Message is retrieved. Let's signal this to probe thread.
             */
            pthread_mutex_lock(&pario_probe_sync);
            message_is_processing = 0;
            pthread_cond_signal(&message_retrieved_from_queue);
            pthread_mutex_unlock(&pario_probe_sync);
        }

        /* INFO: IO Data is received in default case statement */

       if(1){

#endif

#ifndef USE_TUNNELFS_PROBE_THREAD
          switch (msg_status.MPI_TAG)
#else
          switch (memfs_service_status.MPI_TAG)
#endif
          {
          case MEMFS_DELEGATED_OPEN:
          case MEMFS_OPEN:
              {
              char *filename;
              int filename_length, access_mode;
              int position = 0;
              int64_t blocksize;
              int pos;

              /* Unpack values of MEMFS_OPEN request */
              LOCK_MPI();
              MPI_Unpack(recv_buf, msg_size, &position, &filename_length, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              filename = malloc(filename_length * sizeof(char));
              MPI_Unpack(recv_buf, msg_size, &position, filename, filename_length, MPI_CHAR,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &access_mode, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &blocksize, 1, ADIO_OFFSET,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &pos, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: received an open request for file %s with amode %d and blocksize %lld\n", 
                      comm_rank, filename, access_mode, blocksize);
#endif

#ifndef USE_TUNNELFS_PROBE_THREAD
              if(msg_status.MPI_TAG == MEMFS_DELEGATED_OPEN) {
#else
              if(memfs_service_status.MPI_TAG == MEMFS_DELEGATED_OPEN) {
#endif
                  /* this is a delegated open request */
                  /* needs to be issued to main thread */
#ifdef DEBUG_THREADS
                  fprintf(stderr, "[s%d]: TAG = MEMFS_DELEGATED_OPEN, request from %d\n", comm_rank, msg_status.MPI_SOURCE);
#endif
                  LOCK_MPI();
                  ADIO_File file;
                  file = (ADIO_File) ADIOI_Malloc(sizeof(struct ADIOI_FileD));
                      
                  pos = -1;
                  
                  int error = 0;
                  MPI_Info info;
                  MPI_Info_create(&info);
                  char block[20];

                  sprintf(block, "%lld", blocksize);
                  MPI_Info_set(info, "blocksize", block);
                  UNLOCK_MPI();

                  file->filename = filename;
                  file->info = info;
                  file->access_mode = access_mode;
                  

                  thread_comm_io(MEMFS_DELEGATED_OPEN, file, NULL, &error, msg_status.MPI_SOURCE, 
                                 (MPI_Datatype) NULL, 0, (ADIO_Offset) NULL, NULL, NULL, (void *) pos);
                  
#ifdef DEBUG_THREADS
                  fprintf(stderr, "[s%d]: finished handling of MEMFS_DELEGATED_OPEN\n", comm_rank);
#endif


              } else {
                  /* This is a "normal" open request */
                  int error = 0;
                  int fd_sys;
                  fd_sys = open_file(filename, access_mode, &error, blocksize, pos);
                  free(filename);

                  /* Pack values of reply message */
                  LOCK_MPI();
                  MPI_Pack_size(2, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
                  UNLOCK_MPI();
                  tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);


                  LOCK_MPI();
                  position = 0;
                  MPI_Pack(&fd_sys, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  MPI_Pack(&error, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  UNLOCK_MPI();

#ifdef DEBUG_THREADS
                  fprintf(stderr, "[s%d]: MEMFS_OPEN: Replying with tag %d, fd_sys: %d, error: %d\n", 
                          comm_rank, MEMFS_REPLY, fd_sys, error);
#endif

                  /* Send MEMFS_REPLY message to source of request */
                  ptMPI_Send(send_buf, position, MPI_PACKED,
                            msg_status.MPI_SOURCE, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);
#ifdef DEBUG_THREADS
                 fprintf(stderr, "[s%d]: finished handling of MEMFS_OPEN\n", comm_rank);
#endif
              }
              break;
              } /* case */
          case MEMFS_CLOSE:
              {
              int position = 0;
              int fd_sys = 0;

              LOCK_MPI();
              MPI_Unpack(recv_buf, msg_size, &position, &fd_sys, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();
#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: received a close request for file %d\n",
                      comm_rank, fd_sys);
#endif

              int error = 0;
              close_file(fd_sys, &error);

              LOCK_MPI();
              MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
              UNLOCK_MPI();
              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);


              LOCK_MPI();
              position = 0;
              MPI_Pack(&error, 1, MPI_INT, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: MEMFS_CLOSE: Answering with tag %d, error: %d\n", comm_rank, MEMFS_REPLY, error);
#endif

              ptMPI_Send(send_buf, position, MPI_PACKED,
                        msg_status.MPI_SOURCE, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);

              break;
              } /* case */
          case MEMFS_READCONT:
              {
              ADIO_Offset global_end_at;
              int fd_sys;
              int position = 0;
              ADIO_Offset offset_at_server;
              ADIO_Offset offset;
              int datatype_size;
              int file_ptr_type;
              int local_blocks;
              int block_count;
              ADIO_Offset send_buf_offset = 0;
              int fp_sys_posn;
              int fp_ind;
              int i, err;
              ADIO_Offset local_end_at;
              int last_server;
              int last_block;
              int mutex_error;
              int msg_id;


              #ifdef MEMFS_TIME
              t1 = gettime();
              #endif

              LOCK_MPI();
#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: MEMFS_READCONT: unpacking values of MEMFS_READCONT\n", comm_rank);
#endif
              MPI_Unpack(recv_buf, msg_size, &position, &msg_id, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &fd_sys, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &datatype_size, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &file_ptr_type, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &global_end_at, 1, ADIO_OFFSET,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &local_blocks, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &offset_at_server, 1, ADIO_OFFSET,
                         MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();
        
#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: MEMFS_READCONT: msg_id: %d, global_end_at: %lld, memfs_num_servers: %d\n", 
                      comm_rank, msg_id, global_end_at, memfs_num_servers);

#endif
              /* regard data distribution between all servers */
              local_end_at = (global_end_at / memfs_num_servers);
              /* round up to full blocksize */
              if((local_end_at % MEMFS_BLOCKSIZE) > 0)
                  local_end_at += MEMFS_BLOCKSIZE - (local_end_at % MEMFS_BLOCKSIZE);

              /* computation of last involved server */
              last_block = global_end_at / MEMFS_BLOCKSIZE;
              if(global_end_at % MEMFS_BLOCKSIZE > 0) {
                  last_block++;
              }
              last_server = (last_block - 1) % memfs_num_servers;
//              last_server = last_block % memfs_num_servers;

              /* last server eventually only reads a part of the last block */
              if(last_server == comm_rank && (global_end_at % MEMFS_BLOCKSIZE) > 0) {
                  /* this is the server which holds the last block of the request */
                  local_end_at -= MEMFS_BLOCKSIZE - (global_end_at % MEMFS_BLOCKSIZE);
              }

              /* all servers "behind" last server read one block less */
              if(comm_rank > last_server) {
                  local_end_at -= MEMFS_BLOCKSIZE;
              }
              assert(local_end_at - offset_at_server >= 0);
             
              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, local_end_at - offset_at_server);

#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: MEMFS_READCONT: local_end_at: %lld, offset_at_server: %lld, local_blocks: %d\n", 
                      comm_rank, local_end_at, offset_at_server, local_blocks);
#endif

              if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
                  #ifdef MEMFS_TIME
                  t3 = gettime();
                  #endif
                  mutex_error = pthread_mutex_lock(&filesystem_mutex);
                  #ifdef MEMFS_TIME
                  t4 = gettime();
                  settime(FS_LOCK, t4-t3);
                  #endif

                  err = memfs_read(fd_sys, offset_at_server, send_buf, local_end_at - offset_at_server);
                  pthread_mutex_unlock(&filesystem_mutex);
                  fp_sys_posn = offset_at_server + err;
              } else { /* read from current location of individual file pointer */
                  offset = fp_ind;
                  fprintf(stderr, "memfs_service: TODO: reading from fp_ind\n");
                  err = memfs_read(fd_sys, offset, send_buf, local_end_at - offset_at_server);
                  fp_ind += err;
                  fp_sys_posn = fp_ind;
              }
#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: MEMFS_READCONT: read complete\n",
                      comm_rank);
#endif

              /* read blocks from file and answer requesting server */
              for(i = 0; i < local_blocks; i++) {
                  if(i == 0) {
                      /* first part can be shorter */
                      block_count = MEMFS_BLOCKSIZE - (offset_at_server % MEMFS_BLOCKSIZE);
                      if(block_count > offset_at_server + local_end_at) {
                          block_count = offset_at_server + local_end_at;
                      }
                  } else {
                      block_count = MEMFS_BLOCKSIZE / datatype_size;
                  }
                  if (i == local_blocks - 1) {
                      /* last part can be shorter */
                      block_count = local_end_at - send_buf_offset - (offset_at_server / datatype_size);
                  }
                  if(block_count > MEMFS_BLOCKSIZE) {
                      /* block_count should not be larger than MEMFS_BLOCKSIZE */
                      block_count = MEMFS_BLOCKSIZE;
                  }
            
#ifdef DEBUG_THREADS
                  fprintf(stderr, "[s%d]: MEMFS_READCONT: Sending %d chars to %d, beginning at offset: %lld\n", 
                          comm_rank, block_count, msg_status.MPI_SOURCE, send_buf_offset);
#endif
                  ptMPI_Send(send_buf + send_buf_offset, block_count, MPI_BYTE,
                             msg_status.MPI_SOURCE, MEMFS_REPLY_IODATA, MPI_COMM_MEMFS_WORLD);

                  send_buf_offset += block_count;
              }

              #ifdef MEMFS_TIME
              t2 = gettime();
              settime(READ_SERVICE, t2 - t1);
              settime(TOTAL_TIME, t2-t1);
              #endif

              break;
              }
           case MEMFS_READSTRIDED:
              {
              break;
              }
           case MEMFS_WRITECONT:
              {
              int fd_sys, ioflag, i;
              MPI_Status io_status;
              int io_size;
              int count, blocks;
              int min_size;
              int datatype_size;
              int file_ptr_type;
              int error[1];
              int global_error[1];
              int fp_sys_posn = 0;
              int fp_ind = 0;
              int len = 0;
              int position = 0;
              int mutex_error;
              ADIO_Offset offset_at_server;
              ADIO_Offset offset;

              #ifdef MEMFS_TIME
              t1 = gettime();
              #endif

              LOCK_MPI();
#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: unpacking values of MEMFS_WRITECONT\n", comm_rank);
#endif
              MPI_Unpack(recv_buf, msg_size, &position, &main_msg_id, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &fd_sys, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
/*
              MPI_Unpack(recv_buf, msg_size, &position, &count, 1, MPI_INT,
                         MPI_COMM_META_REDUCED);
*/
              MPI_Unpack(recv_buf, msg_size, &position, &datatype_size, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &file_ptr_type, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &blocks, 1, MPI_INT, 
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &offset_at_server, 1, ADIO_OFFSET,
                         MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();
/*
              fprintf(stderr, "memfs_service: VALUES: datype_size: %d, file_ptr_type: %d, offset: %lld, blocks: %d\n", 
                      datatype_size, file_ptr_type, offset, blocks);
*/
#ifdef DEBUG_THREADS  
              fprintf(stderr, "[s%d]: MEMFS_WRITECONT: main_msg_id: %d numblocks: %d, offset_at_server: %lld, fd_sys: %d\n", 
                      comm_rank, main_msg_id, blocks, offset_at_server, fd_sys);
#endif
 
              *global_error = 0;
              /* receive all I/O Blocks */
              for(i = 0; i < blocks; i++) 
              {

#ifndef USE_TUNNELFS_PROBE_THREAD
                  while(1) {
                      LOCK_MPI();
                      ioflag = 0;
                      /* wait for incoming I/O Data */
                      MPI_Iprobe(msg_status.MPI_SOURCE, MEMFS_IODATA, MPI_COMM_MEMFS_WORLD, &ioflag, &io_status);
                      if(ioflag) {
#ifdef DEBUG_THREADS
                          fprintf(stderr, "[s%d]: Received I/O data block %d in MEMFS_WRITECONT! total blocks: %d\n", 
                                  comm_rank, i+1, blocks);
#endif
                          MPI_Get_count(&io_status, MPI_BYTE, &io_size);
                          UNLOCK_MPI();

                          /* checking bounds of receive buffer */
                          tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, io_size);
                          ptMPI_Recv(recv_buf, io_size, MPI_BYTE,
                                     io_status.MPI_SOURCE, io_status.MPI_TAG,
                                     MPI_COMM_MEMFS_WORLD, &io_status);
#else

                         /* clearing the status field */
                        memset(&memfs_service_status, 0, sizeof(MPI_Status));
                        memfs_service_status_is_empty = 1;

#ifdef DEBUG_THREADS
                              fprintf(stderr, "memfs_service: [%d] waiting for message_for_memfs_service in write\n",
                                      comm_rank, fd_sys);
#endif                              
                        /* wait for message signal */
                        while ((memfs_service_status_is_empty) && !pario_shutdown)
                            pthread_cond_wait(&message_for_memfs_service,
                                              &memfs_service_sync);

                        if (pario_shutdown)
                            break;

                      if (memfs_service_status.MPI_TAG == MEMFS_IODATA)
                      {
                          LOCK_MPI();
                          MPI_Get_count(&memfs_service_status, MPI_BYTE, &msg_size);

                          tunnelfs_adjust_buffer(&recv_buf, &recv_buf_size, msg_size);

                          MPI_Recv(recv_buf, recv_buf_size, MPI_BYTE,
                                   memfs_service_status.MPI_SOURCE,
                                   memfs_service_status.MPI_TAG, MPI_COMM_MEMFS_WORLD,
                                   &msg_status);
                         UNLOCK_MPI();

                     /**
                      * Message is retrieved. Let's signal this to probe thread.
                      */
                         pthread_mutex_lock(&pario_probe_sync);
                         message_is_processing = 0;
                         pthread_cond_signal(&message_retrieved_from_queue);
                         pthread_mutex_unlock(&pario_probe_sync);
                      }

                      io_size = msg_size;


#endif
                          count = io_size / datatype_size;
                          len = datatype_size * count;

                          if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
#ifdef DEBUG_THREADS
                              fprintf(stderr, "[s%d]: MEMFS_WRITECONT: writing to file %d, offset_at_server: %lld, len: %d\n", 
                                      comm_rank, fd_sys, offset_at_server, len);
#endif
                              #ifdef MEMFS_TIME
                              t3 = gettime();
                              #endif
                              mutex_error = pthread_mutex_lock(&filesystem_mutex);
                              #ifdef MEMFS_TIME
                              t4 = gettime();
                              settime(FS_LOCK, t4-t3);
                              #endif

                              *error = memfs_write(fd_sys, offset_at_server, recv_buf, len);
                              pthread_mutex_unlock(&filesystem_mutex);
#ifdef DEBUG_THREADS
                              fprintf(stderr, "[s%d]: MEMFS_WRITECONT: writing to file %d complete\n",
                                      comm_rank, fd_sys);
#endif                              
                              if(*error == -1) *global_error = -1;
                              fp_sys_posn = offset_at_server + *error;
                          } else { /* write from current location of individual file pointer */
                              offset = fp_ind;
                              *error = memfs_write(fd_sys, offset_at_server, recv_buf, len);
                              if(*error == -1) *global_error = -1;
                              fp_ind += *error;
                              fp_sys_posn = fp_ind;
#ifdef DEBUG_THREADS
                              fprintf(stderr, "ADIOI_MEMFS_WriteContig: Writing without ADIO_EXPLICIT_OFFSET\n");
#endif
                          }
                          break;
#ifndef USE_TUNNELFS_PROBE_THREAD
                      } /* if */

                      UNLOCK_MPI();
                  } /* while */
#endif
              /* jump to the beginning of the next block */ 
              offset_at_server += (MEMFS_BLOCKSIZE - (offset_at_server % MEMFS_BLOCKSIZE));
              } /* for */


/*

              LOCK_MPI();
              MPI_Pack_size(2, MPI_INT, TUNNELFS_COMM_WORLD, &min_size);
              UNLOCK_MPI();
              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);

              LOCK_MPI();
              position = 0;
              MPI_Pack(&msg_id, 1, MPI_INT, send_buf, send_buf_size, 
                       &position, TUNNELFS_COMM_WORLD);
              MPI_Pack(&global_error, 1, MPI_INT, send_buf, send_buf_size,
                       &position, TUNNELFS_COMM_WORLD);
              UNLOCK_MPI();


//#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: MEMFS_WRITECONT: id: %d, Sending reply to %d with tag %d, error: %d\n", 
                      comm_rank, msg_id-1, pario_map_w2r(msg_status.MPI_SOURCE), MEMFS_REPLY_ERRORCODE, *error);
//#endif

              ptMPI_Send(send_buf, position, MPI_PACKED,
                        msg_status.MPI_SOURCE, MEMFS_REPLY_ERRORCODE, TUNNELFS_COMM_WORLD);
              fprintf(stderr, "[s%d]: sent message with tag: %d to: %d\n", 
                      comm_rank, MEMFS_REPLY_ERRORCODE, pario_map_w2r(msg_status.MPI_SOURCE));


*/


              #ifdef MEMFS_TIME
              t2 = gettime();
              settime(WRITE_SERVICE, t2-t1);
              settime(TOTAL_TIME, t2-t1);
              #endif

              break;
              }
           case MEMFS_WRITESTRIDED:
              {
              break;
              }
           case MEMFS_RESIZE:
              {
              int position = 0;
              int error = 0;
              int fd_sys;
              ADIO_Offset end_at;

              LOCK_MPI();

              MPI_Unpack(recv_buf, msg_size, &position, &fd_sys, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              MPI_Unpack(recv_buf, msg_size, &position, &end_at, 1, ADIO_OFFSET,
                         MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

              error = memfs_resize(fd_sys, end_at);

              LOCK_MPI();
              MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
              UNLOCK_MPI();
              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);


              LOCK_MPI();
              position = 0;
              MPI_Pack(&error, 1, MPI_INT, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

              ptMPI_Send(send_buf, position, MPI_PACKED,
                        msg_status.MPI_SOURCE, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);

              break;
              }
           case MEMFS_DELETE:
              {
              int position = 0;
              int filename_length;
              char *filename;
              int error = 0;


              LOCK_MPI();

              MPI_Unpack(recv_buf, msg_size, &position, &filename_length, 1, MPI_INT,
                         MPI_COMM_MEMFS_WORLD);
              filename = malloc(filename_length * sizeof(char));
              MPI_Unpack(recv_buf, msg_size, &position, filename, filename_length, MPI_CHAR,
                         MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();
#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: received a delete request for file %s, filename_length: %d\n",
                      comm_rank, filename, filename_length);
#endif

              delete_file(filename, &error);

              LOCK_MPI();
              MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
              UNLOCK_MPI();
              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);


              LOCK_MPI();
              position = 0;
              MPI_Pack(&error, 1, MPI_INT, send_buf, send_buf_size,
                       &position, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

#ifdef DEBUG_THREADS
              fprintf(stderr, "[s%d]: MEMFS_DELETE: Answering with tag %d, error: %d\n", comm_rank, MEMFS_REPLY, error);
#endif

              ptMPI_Send(send_buf, position, MPI_PACKED,
                        msg_status.MPI_SOURCE, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);

              break;
              }
           case MEMFS_FCNTL:
              {
              break;
              }
           case MEMFS_SEEKIND:
              {
              break;
              }
           case MEMFS_SETINFO:
              {
              break;
              }
      case MEMFS_SETLOCK:
              {
          int position;
          int fh,i;  
          int size;          /* Size of blocks[] that need to be locked */     
          int ADIO_Offset_size;
          int comm_rank;          

#ifdef DEBUG_LOCKS
              MPI_Comm_rank(MPI_COMM_MEMFS_WORLD, &comm_rank);
              fprintf(stderr, "service thread [%d] MEMFS_SETLOCK \n",comm_rank);
#endif
          position = 0;
          LOCK_MPI();
          MPI_Unpack(recv_buf, msg_size, &position, &fh, 1, MPI_INT,
            MPI_COMM_MEMFS_WORLD);
#ifdef DEBUG_LOCKS
              fprintf(stderr, "service thread [%d] 1. MPI_Unpack() \n",comm_rank);
#endif
          MPI_Unpack(recv_buf, msg_size, &position, &size, 1, MPI_INT,
            MPI_COMM_MEMFS_WORLD);

#ifdef DEBUG_LOCKS
              fprintf(stderr, "service thread [%d] 2. MPI_Unpack() \n",comm_rank);
#endif
          //MPI_Type_size(ADIO_OFFSET, &ADIO_Offset_size);

#ifdef DEBUG_LOCKS
              fprintf(stderr, "service thread [%d] MPI_Type_size() \n",comm_rank);
#endif
          int *blocks = malloc(size * sizeof(int));  /* Blocks of a file, that need to be locked */

#ifdef DEBUG_LOCKS
              fprintf(stderr, "service thread [%d] blocks malloc(), size %d, position %d \n",comm_rank, size, position);
#endif
              MPI_Unpack(recv_buf, msg_size, &position, blocks, size, MPI_INT,
            MPI_COMM_MEMFS_WORLD);

#ifdef DEBUG_LOCKS
              fprintf(stderr, "service thread [%d] 3. MPI_Unpack() \n",comm_rank);
#endif
          UNLOCK_MPI();

#ifdef DEBUG_LOCKS
             fprintf(stderr, "service thread [%d] MEMFS_SETLOCK: \n",comm_rank);
#endif
          
          /* 
           * Lock blocks of file in local filetable 
           * -call function lock_file() defined in ad_memfs_files.h
           * If try lock could not lock complete file, add lock_file() to queue
           * TODO: implement queue in service thread()
           */
          int num_of_server = msg_status.MPI_SOURCE;
          int error = 0;
              error = memfs_lock_file(fh, blocks, size, num_of_server);

          /* If the block could not be locked add the request information into
           * a queue and repeat the request after the blocks have been unlocked.
           */ 
          if(error == -1){
        v.fh = fh;
                v.size = size;
                v.server = num_of_server;
                v.blocks = blocks;
            // add element to queue
                //fprintf(stderr, "service thread [%d] Add element to queue \n", comm_rank);
                queueEnqueue(q,v);
            queue_size++;
        //queuePrint(q);
        //fprintf(stderr," \n");

        //free(blocks);
            break;
          }
        
          /* memfs_lock_file() worked, blocks are all locked,
               * send reply to server that requested the blocks to be locked
               */
          if(error == 1){
            /* 
             * ptMPI_Send answer, that file blocks are locked
             */ 
            LOCK_MPI();
            MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
            UNLOCK_MPI();
        
            tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
    
            LOCK_MPI();
            position = 0;
            MPI_Pack(&error, 1, MPI_INT, send_buf, send_buf_size, 
             &position, MPI_COMM_MEMFS_WORLD);
            UNLOCK_MPI();

                ptMPI_Send(send_buf, position, MPI_PACKED, 
               num_of_server, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);

                free(blocks);
            break;
               }

            //free(blocks);
            break;
              }
           case MEMFS_REMOVELOCK:
              {
              int position=0;
              int fh;
              int size;              /* Size of blocks[] that need to be unlocked */
          int i,j;
          int *unlock_blocks = NULL;

          //fprintf(stderr, "MEMFS_REMOVELOCK [%d] \n", comm_rank);

              LOCK_MPI();
              //fprintf(stderr, "MEMFS_REMOVELOCK [%d] LOCK_MPI()\n", comm_rank);
              MPI_Unpack(recv_buf, msg_size, &position, &fh, 1, MPI_INT,
                        MPI_COMM_MEMFS_WORLD);

              //fprintf(stderr, "MEMFS_REMOVELOCK [%d] MPI_Unpack() 1.\n", comm_rank);
              MPI_Unpack(recv_buf, msg_size, &position, &size, 1, MPI_INT,
                        MPI_COMM_MEMFS_WORLD);

              //fprintf(stderr, "MEMFS_REMOVELOCK [%d] MPI_Unpack() 2.\n", comm_rank);

          if(unlock_blocks == NULL){
            //fprintf(stderr, "MEMFS_REMOVELOCK [%d] NULL MALLOC, size=%d \n", comm_rank,size);      
        unlock_blocks = (int*)malloc(size * sizeof(int));  /* Blocks of a file, that need to be unlocked */
              } else {
                //fprintf(stderr, "MEMFS_REMOVELOCK [%d] NOT NULL MALLOC \n", comm_rank);
          }

              MPI_Unpack(recv_buf, msg_size, &position, unlock_blocks, size, MPI_INT,
                        MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

              //fprintf(stderr, "MEMFS_REMOVELOCK [%d] MPI_Unpack() 3. \n", comm_rank);


              /*
               * Unlock blocks of file in local filetable
               * -call function unlock_file() defined in ad_memfs_lock.h
               * If unlock could not lock complete file, add unlock_file() to queue
               * TODO: implement queue in service thread()
               */
              int error = 0;
              error = memfs_unlock_file(fh, unlock_blocks, size, msg_status.MPI_SOURCE);
              if(error < 0){
                fprintf(stderr, "memfs_unlock_file error, Filehandle: %d, Source: %d \n", fh, msg_status.MPI_SOURCE);
              }


              /*
               * ptMPI_Send answer, that file blocks are locked
               */
              LOCK_MPI();
              MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
              UNLOCK_MPI();

              tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);

              LOCK_MPI();
              position = 0;
              MPI_Pack(&error, 1, MPI_INT, send_buf, send_buf_size,
                        &position, MPI_COMM_MEMFS_WORLD);
              UNLOCK_MPI();

              ptMPI_Send(send_buf, position, MPI_PACKED,
                        msg_status.MPI_SOURCE, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);


          /* Check if queue is empty. (1==true, 0==false)
           * True  => break
               * False => read element, memfs_lock_file()
           */ 
          if( queueIsEmpty(q) == 1){
        //fprintf(stderr,"MEMFS_REMOVELOCK: queue is empty \n");
            //free(blocks);
        break;
              }
    
          for(i = 0; i < queue_size; i++){
        // get first element ou of queue
                v = queueDequeue(q);
        
        //fprintf(stderr,"Queue: fh %d, size %d, server %d, blocks=%d \n", v.fh, v.size, v.server, v.blocks[0]);
        //for(j=0; j < v.size; j++){
        //  fprintf(stderr,"[%d]", v.blocks[j]);
        //}
        //fprintf(stderr,"\n");
            
        // try lock file  
                error = memfs_lock_file(v.fh, v.blocks, v.size, v.server);

        if(error == 1){
                  /*
                   * ptMPI_Send answer, that file blocks are locked
                   */
          queue_size--;
              min_size = 0;

                  LOCK_MPI();
                  MPI_Pack_size(1, MPI_INT, MPI_COMM_MEMFS_WORLD, &min_size);
                  UNLOCK_MPI();

                  tunnelfs_adjust_buffer(&send_buf, &send_buf_size, min_size);
 
                  LOCK_MPI();
                  position = 0;
                  MPI_Pack(&error, 1, MPI_INT, send_buf, send_buf_size,
                           &position, MPI_COMM_MEMFS_WORLD);
                  UNLOCK_MPI();
   
                  ptMPI_Send(send_buf, position, MPI_PACKED,
                             v.server, MEMFS_REPLY, MPI_COMM_MEMFS_WORLD);
          //free(blocks);
        }

        if(error == -1){
          /* put information back into the queue */
          queueEnqueue(q,v);         
        }
               }

              free(unlock_blocks);
           }

          } /* switch */
      } else {
          ;
#ifndef USE_TUNNELFS_PROBE_THREAD
          UNLOCK_MPI();
#endif
      }
#ifdef DEBUG_THREADS
          if(output == 0) {
              fprintf(stderr, "[s%d]: Waiting for request\n", comm_rank);
              output = 1;
          }
#endif


      pthread_mutex_lock(&shutdown_mutex);
      if(memfs_shutdown) {
          pthread_mutex_unlock(&shutdown_mutex);
          break;
      }
      pthread_mutex_unlock(&shutdown_mutex);


    } /* while */
    free(recv_buf);
    free(send_buf); 

    pthread_mutex_unlock(&memfs_service_sync);

//#ifdef DEBUG_THREADS
    fprintf(stderr, "[s%d]: is exiting now\n", comm_rank);
//#endif
    pthread_exit((void *) 0);
}

