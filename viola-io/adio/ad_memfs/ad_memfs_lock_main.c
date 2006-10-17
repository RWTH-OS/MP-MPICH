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
* File:         ad_memfs_lock_main.c                                      * 
* Description:                                                            *
* Author(s):    Marcel Birkner <Marcel.Birkner@fh-bonn-rhein-sieg.de>     * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/ 

#include "ad_memfs.h"
#include "ad_memfs_lock.h"
#include "ad_memfs_main.h"
#include "ad_tunnelfs_msg.h"


        int send_buf_size_lock = 0;
        void *send_buf_lock = NULL;
        int recv_buf_size_lock = 0;
        void *recv_buf_lock = NULL;

/*
    Wait for reply of other Memfs Server
    Accept messages with tag range 0x4000 - 0x4fff
    Receive reply in buffer recv_buf
*/
int get_lock_reply(MPI_Status *msg_status, int *msg_size, int tag) {
    int flag;
    int comm_rank;
    int wait_for_reply = 1;

#ifdef DEBUG_LOCKS
	MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);
	fprintf(stderr, "lock_main() [%d] get_lock_reply() 1. \n",comm_rank);
#endif

    while(wait_for_reply) {
        LOCK_MPI();
        MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_META_REDUCED, &flag, msg_status);
/*        if(flag && msg_status->MPI_TAG >= 0x4000 && msg_status->MPI_TAG <= 0x4fff) */

        if(flag && msg_status->MPI_TAG == tag)
        {
#ifdef DEBUG_THREADS
            fprintf(stderr, "lock_main: [%d] reply received from rank %d with tag: %d\n",
                    msg_status->MPI_SOURCE, msg_status->MPI_TAG);
#endif
            /* Retrieving size of next message */
            if(tag == MEMFS_IODATA || tag == MEMFS_REPLY_IODATA)
                MPI_Get_count(msg_status, MPI_BYTE, msg_size);
            else
                MPI_Get_count(msg_status, MPI_PACKED, msg_size);
            UNLOCK_MPI();
            tunnelfs_adjust_buffer(&recv_buf_lock, &recv_buf_size_lock, *msg_size);
            if(tag == MEMFS_IODATA || tag == MEMFS_REPLY_IODATA)
                ptMPI_Recv(recv_buf_lock, recv_buf_size_lock, MPI_BYTE,
                           msg_status->MPI_SOURCE, msg_status->MPI_TAG,
                           MPI_COMM_META_REDUCED, msg_status);
            else
                ptMPI_Recv(recv_buf_lock, recv_buf_size_lock, MPI_PACKED,
                           msg_status->MPI_SOURCE, msg_status->MPI_TAG,
                           MPI_COMM_META_REDUCED, msg_status);
            wait_for_reply = 0;
         } else {
            UNLOCK_MPI();
         }
    }
#ifdef DEBUG_LOCKS
        fprintf(stderr, "lock_main() [%d] get_lock_reply() 2. \n",comm_rank);
#endif

    //free(send_buf_lock);
    //free(recv_buf_lock);

    return 0;
}




void set_lock(int fh, int *blocks, int size, int num_server, int *error)
{
	int destination, position, min_size, pack_size;
 	//int send_buf_size = 0;
	//void *send_buf = NULL;	
	//int recv_buf_size = 0;
	//void *recv_buf = NULL;
	int reply_size;
	MPI_Status reply_status;
	int *remote_error;
	int comm_rank,i;


#ifdef DEBUG_LOCKS
	MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);
        //for(i=0; i<size; i++){
          fprintf(stderr, "lock_main() [%d] set_lock() from block [%d] to [%d] \n",comm_rank, blocks[0], blocks[size-1]);
        //}
#endif

 	remote_error =(int *) malloc (sizeof(int));

	/* 
	 * -calculate MEMFS Server that is responsible for the file
	 *  server = filehandle % num_server;
         */
	destination = fh % num_server;

#ifdef DEBUG_LOCKS
		fprintf(stderr,"lock_main() [%d] set_lock() destination %d, fh %d, num_server %d, blocks %d, size %d \n",comm_rank,destination, fh, num_server, *blocks, size); 
#endif
	

	/* 
	 * -Initialize communication with other MEMFS Server, set variables
	 * -use MPI_Pack_size() to calculate buffer size for the communication
	 *  Parameter: fh, blocks[]

	 * -use tunnelfs_adjust_buffer() to allocate the required buffer
	 * -use MPI_Pack() to put all variables into the buffer
	 * -use ptMPI_Send() to send the request to the responsible MEMFS Server
	 *  5th parameter is the FLAG=MEMFS_SETLOCK
	 */
	position  = 0;
	pack_size = 0;

	LOCK_MPI();
	min_size = 0;
	MPI_Pack_size(2, MPI_INT, MPI_COMM_META_REDUCED, &pack_size);
	min_size += pack_size;
        MPI_Pack_size(size, MPI_INT, MPI_COMM_META_REDUCED, &pack_size);
        min_size += pack_size;
	UNLOCK_MPI();
	tunnelfs_adjust_buffer(&send_buf_lock, &send_buf_size_lock, min_size);
	
	LOCK_MPI();
	MPI_Pack(&fh, 1, MPI_INT, send_buf_lock, send_buf_size_lock, 
		&position, MPI_COMM_META_REDUCED);
	MPI_Pack(&size, 1, MPI_INT, send_buf_lock, send_buf_size_lock,
		&position, MPI_COMM_META_REDUCED);
	MPI_Pack(blocks, size, MPI_INT, send_buf_lock, send_buf_size_lock, 
		&position, MPI_COMM_META_REDUCED);
	UNLOCK_MPI();

	#ifdef DEBUG_LOCKS
	MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);
	fprintf(stderr,"lock_main() [%d] set_lock(): min_size %d, destination %d \n",comm_rank,min_size,destination);
	#endif

        #ifdef DEBUG_LOCKS
        fprintf(stderr, "lock_main() [%d] set_lock(): Try ptMPI_Send \n", comm_rank);
        #endif

	ptMPI_Send(send_buf_lock, position, MPI_PACKED, destination, MEMFS_SETLOCK, 
		MPI_COMM_META_REDUCED);

        #ifdef DEBUG_LOCKS
        fprintf(stderr, "lock_main() [%d] set_lock(): ptMPI_Send successful \n", comm_rank);
        #endif

	/*
	 * -use get_reply() to wait for the result of ptMPI_Send()
	 * -use MPI_Unpack() to get all variables from the buffer
	 */
	get_lock_reply(&reply_status, &reply_size, MEMFS_REPLY);

        #ifdef DEBUG_LOCKS
        fprintf(stderr, "lock_main() [%d] set_lock() get_lock_reply() successfull \n", comm_rank);
        #endif

	position = 0;
	LOCK_MPI();
	MPI_Unpack(recv_buf_lock, reply_size, &position, 
		&(remote_error[reply_status.MPI_SOURCE]),
		1, MPI_INT, MPI_COMM_META_REDUCED);
	UNLOCK_MPI();

	#ifdef DEBUG_LOCKS
		fprintf(stderr, "lock_main() [%d] set_lock(): destination %d, Error Code: %d \n",comm_rank,destination,remote_error[reply_status.MPI_SOURCE]);
	#endif

	/* 
	 * TODO:
	 * -if an error occurred:
         *  *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
         *           myname, "I/O Error", "%s", strerror(errno));
	 * -else
	 *  *error = MPI_SUCCESS;
	 */


	/* Free Buffer */
	//free(send_buf_lock);
	//free(recv_buf_lock);
return;

}

void remove_lock(int fh, int *blocks, int size, int num_server, int *error)
{
        int destination, position, min_size, pack_size;
        //int send_buf_size = 0;
        //void *send_buf = NULL;
        //int recv_buf_size = 0;
        //void *recv_buf = NULL;
        int reply_size;
        MPI_Status reply_status;
        int remote_error[1];
        int comm_rank;

	MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);

        /*
         * -calculate MEMFS Server that is responsible for the file
         *  server = filehandle % num_server;
         */
        destination = fh % num_server;

        #ifdef DEBUG_LOCKS
                fprintf(stderr,"remove_lock [%d]: destination %d, fh %d, num_server %d \n",comm_rank, destination, fh, num_server);
        #endif


        /*
         * TODO:
         * -Initialize communication with other MEMFS Server, set variables
         * -use MPI_Pack_size() to calculate buffer size for the communication
         * -use tunnelfs_adjust_buffer() to allocate the required buffer
         * -use MPI_Pack() to put all variables into the buffer
         * -use ptMPI_Send() to send the request to the responsible MEMFS Server
	 *  5th parameter is the FLAG=MEMFS_REMOVELOCK
         */
        position  = 0;
        pack_size = 0;

        LOCK_MPI();
        min_size = 0;
        MPI_Pack_size(2, MPI_INT, MPI_COMM_META_REDUCED, &pack_size);
        min_size += pack_size;
        MPI_Pack_size(size, MPI_INT, MPI_COMM_META_REDUCED, &pack_size);
        min_size += pack_size;
        UNLOCK_MPI();
        tunnelfs_adjust_buffer(&send_buf_lock, &send_buf_size_lock, min_size);

        #ifdef DEBUG_LOCKS
                fprintf(stderr,"remove_lock [%d]: MPI_Pack_size \n",comm_rank);
        #endif


        LOCK_MPI();
        MPI_Pack(&fh, 1, MPI_INT, send_buf_lock, send_buf_size_lock,
                &position, MPI_COMM_META_REDUCED);
        MPI_Pack(&size, 1, MPI_INT, send_buf_lock, send_buf_size_lock,
                &position, MPI_COMM_META_REDUCED);
        MPI_Pack(blocks, size, MPI_INT, send_buf_lock, send_buf_size_lock,
                &position, MPI_COMM_META_REDUCED);
        UNLOCK_MPI();

        #ifdef DEBUG_LOCKS
                fprintf(stderr,"remove_lock [%d]: MPI_Pack \n",comm_rank);
        #endif


        ptMPI_Send(send_buf_lock, position, MPI_PACKED, destination, MEMFS_REMOVELOCK,
                MPI_COMM_META_REDUCED);

        /*
         * TODO:
         * -use get_reply() to wait for the result of ptMPI_Send()
         * -use MPI_Unpack() to get all variables from the buffer
         */
        #ifdef DEBUG_LOCKS
                fprintf(stderr,"remove_lock [%d]: get_reply \n",comm_rank);
        #endif
        get_reply(&reply_status, &reply_size, MEMFS_REPLY);

        #ifdef DEBUG_LOCKS
                fprintf(stderr,"remove_lock [%d]: MPI_Unpack \n",comm_rank);
        #endif
        position = 0;
        LOCK_MPI();
        MPI_Unpack(recv_buf_lock, reply_size, &position,
                &(remote_error[reply_status.MPI_SOURCE]),
                1, MPI_INT, MPI_COMM_META_REDUCED);
        UNLOCK_MPI();

        #ifdef DEBUG_LOCKS
                fprintf(stderr,"remove_lock [%d]: MPI_UNLOCK_MPI() worked \n",comm_rank);
        #endif


        /*
         * TODO:
         * -if an error occurred:
         *  *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
         *           myname, "I/O Error", "%s", strerror(errno));
         * -else
         *  *error = MPI_SUCCESS;
         */


        /* Free Buffer */
        //free(send_buf_lock);
        //free(recv_buf_lock);

return;

}
