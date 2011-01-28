/* 
 * $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 * 
 * 
 * 
 * This code implements the routing process for the MPI metacomputing project.
 * The process is a MPI process which gets messages from the local host via a
 * special ADI Device (the gateway ADI) and forwards them via TCP/IP or AAL5 to
 * another router on a remote metahost. 
 * 
 */

#undef TEST_SVC
/*#define TEST_TCP*/
/* #define META_DEBUG */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#ifdef ROUTER_THREADS
#include <pthread.h>
#endif


#include "mpi.h"
#include "mpiimpl.h"
#include "connection.h"

#include "mpi_router.h"
#include "conn_common.h"

#include "mpid.h"

#ifndef WIN32
#include "perfserver.h"
#endif

#include "../../mpid/ch2/packets.h"

#include <metaconfig.h>
#include "rhlist.h"
#include "prof_timer.h"
#include "reqalloc.h"
#include "sendq.h"

#define ROUTER_ABORT_INIT {PRERROR("Router Init failed -Router Process calls local MPI_Abort\n");MPI_Barrier(MPI_COMM_HOST);MPI_Abort(MPI_COMM_HOST, 99); exit(-1);}
#define ROUTER_ABORT {PRERROR("Router Init failed -Router Process calls local MPI_Abort\n");MPI_Abort(MPI_COMM_HOST, 99); exit(-1);}

extern char * MPIR_process_name; /* lives in src/env/initutil.c */

/* only relevant for heterogenous configurations
   (use switch HETERO in the configfile!) */
int MPID_router_byte_order;

char            configfile[256];

struct RouterConfig MPIR_RouterConfig;

/* threads handling the sockets */
int             nbr_threads;
pthread_t       *imp_threads;

extern int      meta_barrier;

/* thread synchronisation */
pthread_mutex_t tcp_lock, mpi_lock;
int             mpi_app_status;
int             *thread_status;

/* performance monitoring */
int perfmon;

/* these are the buffers in which the router receives messages from application
   processes on its metahost that are sent via non-blocking sends with the
   rendezvous protocol; the second array holds the length of the buffers,
   the third array indicates if a meta header has already been sent */
Meta_Header **meta_msg_i;
int *meta_msg_i_size, *meta_header_sent;


/* Byte swap an array of length n of N byte integal elements */
/* A good compiler should unroll the inner loops. Letting the compiler do it
   gives us portability.  Note that we might want to isolate the 
   cases N = 2, 4, 8 (and 16 for long double and perhaps long long)
 */
void  BSwap_N_inplace(b, N, n)
unsigned char *b;
int n, N;
{
  int i, j;
  for (i = 0; i < n*N; i += N)
    for (j = 0; j < N/2; j ++)
      SWAP2(b[i + j], b[i + N - j - 1]);
  
}
/* host_to_exch_int_struct changes an array of integers from
   host to exchange order, if necessary */
void host_to_exch_int_struct(void * b, int size) {
       /* if the host byte order is not equal to that byte order which is used for router
	  communication, we swap bytes */
	if (MPID_router_byte_order != MPIR_RouterConfig.exchangeOrder ) 
		BSwap_N_inplace((unsigned char *) b, sizeof(int), size / sizeof(int));
}
/* get_byte_order determines the byte order of the host
   returns L_ENDIAN or B_ENDIAN */
int get_byte_order(void) {
	int a=1;
	if (*((char*)(&a)) == 1 ) 
		/* little endian */
		return L_ENDIAN;
	else
		return B_ENDIAN;
}


char *adjustbuffer(char *buf, size_t actualsize, size_t newsize)
{
    if (buf == NULL) {
	PRERROR1("adjustbuffer: buffer pointer NULL, allocating %d bytes\n", newsize);
	buf = (char *) malloc (newsize);
    } else {
	if (actualsize < newsize)
	    buf = (char *) realloc(buf, newsize);
    }
    
    if (buf == NULL) {
	PRERROR("adjustbuffer: realloc failed\n");
	ROUTER_ABORT;
    }
    
    return buf;
}


int
usage(void)
{
    printf(" Options: -r configfile\n");
    return -1;
}


/*
 * sync_connection
 * 
 * this function synchronizes the established connections and thus the corresponding 
 * routing processes
 */
void sync_connection(int conn) {
   Meta_Header sync_msg;

   RDEBUG2("%ssyncing connection %d\n", RDEBUG_dbgprefix, conn);
   fflush(stderr);

   sync_msg.mode = ROUTER_MSG; 
   sync_msg.msg.Rout.command = SYNC;
   sync_msg.msg.Rout.arg  = SYNC_REQUEST;

   if (get_conn_type(conn) == CONN_SERVER) {
      /* wait for send-receive consistency in ATM SVC connection */
/*		Sleep(100);*/
       /*      usleep(100000);*/
      while (sync_msg.msg.Rout.arg != SYNC_OK) {
         sync_msg.msg.Rout.src  = CONN_SERVER;
         sync_msg.msg.Rout.dest = CONN_CLIENT;
         sync_msg.msg.Rout.arg  = SYNC_REQUEST;
         
         host_to_exch_int_struct(&sync_msg, sizeof(sync_msg));
         if ( conn_send_message((void *) &sync_msg, sizeof(Meta_Header), conn) < 0) {
            RDEBUG("sync_connection(): error sending at SERVER\n");
            ROUTER_ABORT;
         }

         if ( conn_receive_message((void *) &sync_msg, sizeof(Meta_Header), conn) < 0) {
            RDEBUG("sync_connection(): error receiving at SERVER\n");
            ROUTER_ABORT;
         }
         host_to_exch_int_struct(&sync_msg, sizeof(sync_msg));
         
         if ((sync_msg.mode != ROUTER_MSG) || (sync_msg.msg.Rout.command != SYNC)) {
            RDEBUG("sync_connection(): error receiving at SERVER, wrong msg mode\n");
            ROUTER_ABORT;
         }
         
         if ((sync_msg.msg.Rout.arg != SYNC_NOTYET) &&
             ((sync_msg.msg.Rout.src != CONN_CLIENT) || 
              (sync_msg.msg.Rout.dest != CONN_SERVER))) {
            RDEBUG2("%ssync_connection(): error in reply on conn %d at SERVER\n", RDEBUG_dbgprefix, conn);
            ROUTER_ABORT;
         }
      }
   } else {
      if ( conn_receive_message((void *) &sync_msg, sizeof(Meta_Header), conn) < 0) {
         RDEBUG("sync_connection(): error receiving at CLIENT\n");
         ROUTER_ABORT;
      }
      host_to_exch_int_struct(&sync_msg, sizeof(sync_msg));
      
      if ((sync_msg.mode != ROUTER_MSG) || (sync_msg.msg.Rout.command != SYNC_REQUEST) || 
          (sync_msg.msg.Rout.src != CONN_SERVER) || (sync_msg.msg.Rout.dest != CONN_CLIENT)) {
         RDEBUG1("%ssync_connection(): error syncing at CLIENT\n", RDEBUG_dbgprefix);
         ROUTER_ABORT;
      }
      sync_msg.msg.Rout.arg  = SYNC_OK;
      sync_msg.msg.Rout.src  = CONN_CLIENT;
      sync_msg.msg.Rout.dest = CONN_SERVER;
      
      host_to_exch_int_struct(&sync_msg, sizeof(sync_msg));
      if ( conn_send_message((void *) &sync_msg, sizeof(Meta_Header), conn) < 0) {
         RDEBUG("sync_connection(): error sending at CLIENT\n");
         ROUTER_ABORT;
      }
   }
   
   RDEBUG2("%sconnection %d synchronized.\n", RDEBUG_dbgprefix, conn);
   return;
}



/* open_connections configures the connections to all remote routers
   and establishes them */
int open_connections()
{
   int             id;
   char ip1[IP_STRING_LENGTH], ip2[IP_STRING_LENGTH];
   int port1, port2;
   int pos;
   struct sockaddr_in addr1, addr2;
   int mode=CONN_CLIENT;

   /*
    * all routers try to connect to lower id's in ascending order the
    * router with id 0 won't enter the loop because it is server for all
    * connections
    */

   for (id = 0; id < rh_getNumRouters(); id++) { /* for all routers on the metahost */
	   Snic *myNic,*otherNic;
	   pos = 0;
	   
	   /* make connections only for my local router, there may be others on the metahost */
	   if (routerlist[id].metahostrank == MPIR_meta_cfg.my_rank_on_metahost) {
	     myNic=routerlist[id].localNicList;
	     otherNic=routerlist[id].remoteNicList;
	     while (myNic) {
	       if ( ! otherNic ) {
		 PRERROR("open_connections: error reading connection endpoint address\n");
		 ROUTER_ABORT;
	       }
	       /* check if the address types matches */
	       if ( myNic->nicType != otherNic->nicType ){
		 PRERROR("open_connections: endpoint address types must be equal!\n");
		 ROUTER_ABORT;
	       }
	       if (rh_getMetahostId(MPIR_meta_cfg.my_metahostname) < routerlist[id].host) mode=CONN_SERV;
	       else mode=CONN_CLIENT;
	       routerlist[id].serv = mode;

	       /* normal IP adresses */
	       if ( myNic->nicType == ADR_TCP ) {
		   memset(&addr1,0,sizeof(struct sockaddr_in));
		   memset(&addr2,0,sizeof(struct sockaddr_in));
		   addr1.sin_addr.s_addr = inet_addr(myNic->nicAddress);
		   addr2.sin_addr.s_addr = inet_addr(otherNic->nicAddress);
		   addr1.sin_family = AF_INET;
		   addr2.sin_family = AF_INET;
		   addr1.sin_port = htons(myNic->port + MPIR_RouterConfig.tcp_portbase);
		   addr2.sin_port = htons(otherNic->port + MPIR_RouterConfig.tcp_portbase);
		   if (pos == 0) { /* first nic */
		       if ((routerlist[id].conn = 
			    conn_add_connection((struct sockaddr *)&addr1, (struct sockaddr *)&addr2, 
					   AF_INET, NULL, mode,  MPIR_RouterConfig.router_timeout)) < 0) {
			   PRERROR2("%s open_connections: error adding connection for id %d\n",RDEBUG_dbgprefix, id);
			   ROUTER_ABORT;
		       }
		   } else if (conn_add_socket(routerlist[id].conn, 
					 (struct sockaddr *)&addr1, (struct sockaddr *)&addr2, NULL) < 0) {
		       PRERROR2("%s open_connections: error adding connection for id %d\n", RDEBUG_dbgprefix, id);
		       ROUTER_ABORT;
		   }
	       }
#ifdef META_ATM
	       if ( (myNic->nicType == ADR_ATM_SVC)|| (myNic->nicType == ADR_ATM_PVC)) {
		   int conn;
		   char host[256];
		   unsigned short sa_family;
/* 		   char *qos = "cbr,aal5:max_pcr=0,min_pcr=10000,sdu=0"; */
		   char *qos = "ubr,aal5:pcr=0,max_sdu=8192";
		   struct sockaddr_atmsvc addr1;
		   struct sockaddr_atmsvc addr2;
		   RDEBUG("==== ATM configuration detected =======\n");
		   sa_family = ( myNic->nicType == ADR_ATM_SVC ) ? PF_ATMSVC : PF_ATMPVC;
		   text2atm(otherNic->nicAddress, (struct sockaddr *) &addr2, sizeof(addr2), T2A_PVC | T2A_SVC | T2A_NAME);
		   text2atm(myNic->nicAddress, (struct sockaddr *) &addr1, sizeof(addr1), T2A_PVC | T2A_SVC | T2A_NAME);

		   if (pos == 0) { /* first nic */
		       if ((routerlist[id].conn = 
			    conn_add_connection((struct sockaddr *) &addr1, (struct sockaddr *) &addr2,
					   sa_family, qos, mode)) < 0 ) {
			   PRERROR1("open_connections: error adding connection for id %d\n", id);
			   ROUTER_ABORT;
		       }
		   } else if (conn_add_socket(routerlist[id].conn, 
			                 (struct sockaddr *)&addr1, (struct sockaddr *)&addr2, NULL) < 0) {
		       PRERROR1("open_connections: error adding connection for id %d\n", id);
		       ROUTER_ABORT;
		   }
	       }
#endif
	       PRVERBOSE4("added connection from %s:%d to %s:%d\n", myNic->nicAddress, myNic->port+MPIR_RouterConfig.tcp_portbase , otherNic->nicAddress, otherNic->port+MPIR_RouterConfig.tcp_portbase);

	       pos++;
	       myNic=myNic->next;
	       otherNic=otherNic->next;
	   } /* while (myNic) */
	   if (conn_establish_connection(routerlist[id].conn) < 0) {
	      PRERROR2("%s open_connections: error establishing connection %d\n", RDEBUG_dbgprefix, routerlist[id].conn);
	       ROUTER_ABORT;
	   } 
       } 
   }
   return 0;
}


/*
 * this function maps the destination rank to a connection in a very static
 * way - with the modulo operator
 */
int
get_conn_for_dest(int dest)
{
    static int      delta;
    
    return (int) metahostlist[MPIR_RouterConfig.otherhostid].np % (int) conn_getnumconn();
}


/* Receive messages from other hosts (via tcp/ip) and MPI_send them internally
   to the recipient process. This function is processed by several threads */
void *import_msgs (void *arg)
{
    MPI_Status *snd_status;
    MPI_Request *snd_request;
    
    IntQueue *availQ;
    IntQueue *pendingQ;
    struct SThreadArgs_import *t_args;
    int i, t_id, conn, dest, req_id, flag, size, found_id;

    char **router_msg; /* array of pointers to buffers for incoming messages */
    int *bufsize;      /* array for actual length of buffers (may be changing during execution) */
    int tmpzero = 0;

    /* set up buffers and other threadspecific data */
    t_args = (struct SThreadArgs_import *)arg;
    t_id        = t_args->thread_id;
    conn        = t_args->connection;
    availQ      = &t_args->availQ;
    pendingQ    = &t_args->pendingQ;
    snd_request = t_args->snd_request;
    snd_status  = t_args->snd_status;
    router_msg  = t_args->router_msg;
    bufsize     = t_args->bufsize;

#ifdef ROUTER_THREADS
    /* threaded version: message import processing loop */
    while (mpi_app_status == MPI_APP_RUNNING) {
	/* timeout of 1 second to periodically check termination of thread */
	if (conn_select_connection(conn, 1000)) {
#else
    /* unthreaded version: no new message -> return */
    if ( ! conn_select_connection(conn, 0) )
	return 0;
#endif
            size = conn_check_message (conn, CHECK_BLOCK);


	    /* while( meta_barrier ); */

	    /* before we get req_id for this transaction (id of buffer to be used), we must make shure that
	       not all buffers are full; if this is the case, we block until at least one buffer is available */
	    while (Qfull (pendingQ)) {
		for (i = Qfirst(pendingQ); i >= 0; i = Qnext(pendingQ)) {
#ifdef ROUTER_THREADS
		    pthread_mutex_lock (&mpi_lock);
#endif
		    MPI_Test(&snd_request[i], &flag, &snd_status[i]);
#ifdef ROUTER_THREADS
		    pthread_mutex_unlock (&mpi_lock);
#endif
		    if (flag) {	
			/* message has been sent */
			Qput (availQ, i);
			Qremove (pendingQ, i);
		    }
		}
	    }

	    /* get id for this transaction */
	    req_id = Qget (availQ);
	    Qput (pendingQ, req_id);
	    
	    router_msg[req_id] = adjustbuffer (router_msg[req_id], bufsize[req_id], size);
	    if( bufsize[req_id] < size )
		bufsize[req_id] = size;
	    if ((size = conn_receive_message(router_msg[req_id], size, conn)) < 0) {
		RDEBUG("import_msgs(): error from receive_message\n");
	    }
	    host_to_exch_int_struct(router_msg[req_id], sizeof(Meta_Header));
	    
	    /* check if other routers are finalizing */
	    if (((Meta_Header *)router_msg[req_id])->mode == ROUTER_MSG) {		    
		switch (((Meta_Header *)router_msg[req_id])->msg.Rout.command) {
		case SYNC:
		    /* reply that we are not ready yet */
#ifdef ROUTER_THREADS
		    pthread_mutex_lock (&tcp_lock);
#endif
		    ((Meta_Header *) router_msg[req_id])->msg.Rout.arg = SYNC_NOTYET;
		    host_to_exch_int_struct(router_msg[req_id], sizeof(Meta_Header));
		    conn_send_message((void *) router_msg[req_id], sizeof(Meta_Header), conn);
#ifdef ROUTER_THREADS
		    pthread_mutex_unlock (&tcp_lock);
#endif
		    
		    Qput (availQ, req_id);
		    Qremove (pendingQ, req_id);
		    break;
		default:
		    PRERROR("import_msgs(): invalid command msg\n");
		    ROUTER_ABORT;
		}
	    } else if (  ((Meta_Header *)(router_msg[req_id]))->msg.MPI.mode == CANCEL   ) {
		/* we received a cancel message */
		MPID_PKT_ANTI_SEND_T *cancel_msg, confirm_msg;
		MPID_PKT_T *cfm;
		MPID_Aint remote_send_id;
		MPIR_SHANDLE *shandle = 0;

		/* preparing confirmation message */
		cancel_msg = (MPID_PKT_ANTI_SEND_T *)( router_msg[req_id] + sizeof( Meta_Header ) );
		MPID_AINT_GET( shandle, cancel_msg->send_id );
		MPID_AINT_SET( remote_send_id, shandle );

		cfm = (MPID_PKT_T *)(&confirm_msg);

		cfm->head.mode         = MPID_PKT_ANTI_SEND_OK;
		confirm_msg.context_id = 0;
		confirm_msg.lrank      = 0;
		confirm_msg.to         = 0;
		confirm_msg.src        = 0;
		confirm_msg.seqnum     = 0;
		confirm_msg.tag        = 0;
		confirm_msg.len        = 0;
		confirm_msg.send_id    = remote_send_id;
		MPID_AINT_SET(confirm_msg.recv_id, &tmpzero);
		
		/* remove cancel message from pending queue */
		Qput (availQ, req_id);
		Qremove (pendingQ, req_id);
		
		/* look up message that is to be cancelled */
		found_id = -1;
		for( i = Qfirst(pendingQ); i >= 0; i = Qnext(pendingQ) ) {
		    if( ((Meta_Header *)(router_msg[i]))->msg.MPI.msgid == ((Meta_Header *)(router_msg[req_id]))->msg.MPI.msgid )
			/* message found in pending queue */
			found_id = i;
		}
		if( found_id == -1 ) {
		    /* message has already been delivered */
		    confirm_msg.cancel = 0;
		}
		else {
		    /* message delivering to destination proc is pending ->cancel via native device */
#ifdef ROUTER_THREADS
		    pthread_mutex_lock( &mpi_lock );
#endif
		    MPI_Cancel( &snd_request[found_id] );
		    MPI_Wait( &snd_request[found_id], &snd_status[found_id] );
		    MPI_Test_cancelled( &snd_status[found_id], &flag );
#ifdef ROUTER_THREADS
		    pthread_mutex_unlock( &mpi_lock );
#endif
		    /* remove message from pending queue */
		    Qput( availQ, found_id );
		    Qremove( pendingQ, found_id );
		    if( flag ) {
			confirm_msg.cancel = 1;
		    }
		    else {
			confirm_msg.cancel = 0;
		    }
		}

		/* send confirm message */
		/* MP lock ? */
		MPI_Send( &confirm_msg, sizeof( MPID_PKT_ANTI_SEND_T ), MPI_BYTE, cancel_msg->src, MPIR_CANCEL_CONFIRM_TAG, MPI_COMM_ALL );

	    }
	    else {
	      if( ((Meta_Header *)(router_msg[req_id]))->msg.MPI.mode == CANCEL_CONFIRM ) {
		  /* this is a cancel confirm message for a process on our metahost
		   * we fake the source of the message with our rank in MPI_COMM_ALL
		   * the reason for this is explained in MPID_Gateway_SendCancelPacket()
		   */
		  ((Meta_Header *)(router_msg[req_id]))->msg.MPI.src_comm_lrank = MPID_MyAllRank;
	      }
	      dest = ((Meta_Header *) router_msg[req_id])->msg.MPI.dest_grank;
		
#ifdef META_DEBUG
	      fprintf (stderr, "import_msgs(): got msg for [a%d] from [m%d], tag %d, MPI size %d\n",
		       dest, ((Meta_Header *) router_msg[req_id])->msg.MPI.src_comm_lrank,
		       ((Meta_Header *) router_msg[req_id])->msg.MPI.tag,
		       ((Meta_Header *) router_msg[req_id])->msg.MPI.count);
	      fflush (stderr);
#endif	    
	      /* XXX check for blocking or nonblocking semantics; if blocking is
		 required, generate reply message */

#ifdef ROUTER_THREADS
	      pthread_mutex_lock (&mpi_lock);
#endif
	      /*  show_global_time("router:sende...");*/
	      /* -------------------------------------------------- */
	      {
		  struct MPIR_DATATYPE     *dtype_ptr;
		  MPIR_SHANDLE             *shandle;
		  static char myname[] = "MPI_ISEND";
		  int mpi_errno;

		  dtype_ptr = MPIR_GET_DTYPE_PTR(MPI_BYTE);

		  MPIR_ALLOCFN(shandle,MPID_Send_alloc,
			       MPIR_COMM_ALL,MPI_ERR_EXHAUSTED,myname );
		  snd_request[req_id] = (MPI_Request)shandle;
		  MPID_Request_init( (MPI_Request)shandle, MPIR_SEND );

		  /* we need the rank of dest in MPI_COMM_ALL in MPID_Gateway_SendCancelPacket(),
		     so we save it here */
		  shandle->partner_grank = MPIR_COMM_ALL->lrank_to_grank[dest];

		  MPIR_REMEMBER_SEND( shandle, router_msg[req_id], size, MPI_BYTE, dest, MPIR_MPIMSG_TAG, MPIR_COMM_ALL);

		  if (dest == MPI_PROC_NULL) {
		      shandle->is_complete = 1;
		  }
		  else
		      /* This COULD test for the contiguous homogeneous case first .... */
		      MPID_IsendDatatype( MPIR_COMM_ALL, router_msg[req_id], size, dtype_ptr, 
					  MPIR_COMM_ALL->local_rank, MPIR_MPIMSG_TAG, 
					  MPIR_ALL_PT2PT_CONTEXT, 
					  MPIR_COMM_ALL->lrank_to_grank[dest], 
					  snd_request[req_id], &mpi_errno, 0 );
	      }

	      /*	      show_global_time("router:gesendet!");*/
#ifdef ROUTER_THREADS
      pthread_mutex_unlock (&mpi_lock);
#endif
	    }
#ifdef ROUTER_THREADS
	    /* unterminating while loop only exists in threaded version */
	}
    }
#endif

    /* wait for completion of pending sends */
    if (!Qempty(pendingQ)) {
	for (i = Qfirst(pendingQ); i >= 0; i = Qnext(pendingQ)) {
#ifdef ROUTER_THREADS
	    pthread_mutex_lock (&mpi_lock);
#endif
	    MPI_Test(&snd_request[i], &flag, &snd_status[i]);
#ifdef ROUTER_THREADS
	    pthread_mutex_unlock (&mpi_lock);
#endif
	    if (flag) {	
		/* message has been sent */
		Qput (availQ, i);
		Qremove (pendingQ, i);
	    }
	}
    }

#ifdef ROUTER_THREADS
    pthread_exit (NULL);
#endif

    return 0;
}

/* Send messages to other hosts (via tcp/ip) after MPI_Recv'ing them internally
   to the recipient process. This function is processed by several threads */
void *export_msgs (void *arg)
{
    MPI_Status recv_status;
    char       **router_msg;
    int        *bufsize;
    int        recv_msgcount;
    int        flag, i;
    int        thrd_err;
    int        t_id, conn, my_comm_host_rank, send_context;
    int        pipe_msg_size_left;
    /* set up buffers and other threadspecific data */
    struct SThreadArgs_export *t_args;
    t_args = (struct SThreadArgs_export *)arg;
    t_id               = t_args->thread_id;
    conn               = t_args->connection;
    router_msg         = t_args->router_msg;
    bufsize            = t_args->bufsize;
    my_comm_host_rank   = t_args->my_comm_host_rank;
    send_context       = t_args->send_context;

    
    
    /* receive messages from the mpi-processes of the localhost 
       and route them to the according host */

#ifdef ROUTER_THREADS
    while (mpi_app_status == MPI_APP_RUNNING) {
	pthread_mutex_lock (&mpi_lock);
#endif
	MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_HOST, &flag, &recv_status);

	if (! flag) {
#ifdef ROUTER_THREADS
	    pthread_mutex_unlock (&mpi_lock);
#ifdef LINUX 
	    pthread_yield();
#endif
#ifdef SOLARIS
	    thr_yield();
#endif
#else
	    /* nonthreaded version exists after IProbe with negative result */
	    return 0;
#endif
	} else {
	    /* get the size of the message to be received */
	    MPI_Get_count(&recv_status, MPI_BYTE, &recv_msgcount);
	    if( (recv_status.MPI_TAG == MPIR_SEPARATE_MSG_TAG) && (meta_header_sent[recv_status.MPI_SOURCE]) ) {
		/* a meta header was sent and this is the message belonging to that header */
		meta_msg_i[recv_status.MPI_SOURCE] = (Meta_Header * )adjustbuffer( (char *)(meta_msg_i[recv_status.MPI_SOURCE]),
										   meta_msg_i_size[recv_status.MPI_SOURCE],
										   recv_msgcount + sizeof( Meta_Header ) );
		if ( meta_msg_i_size[recv_status.MPI_SOURCE] < recv_msgcount + sizeof( Meta_Header ) )
		    meta_msg_i_size[recv_status.MPI_SOURCE] = recv_msgcount + sizeof( Meta_Header );
	    }
	    else {
		router_msg[0] = adjustbuffer(router_msg[0], bufsize[0], recv_msgcount);
		if( bufsize[0] < recv_msgcount )
		    bufsize[0] = recv_msgcount;
	    }
	    
#ifndef WIN32
		/* send msgcount to perfmeter server */
	    SENDTO_PERFSERVER( recv_msgcount );
#endif
	    if( (recv_status.MPI_TAG == MPIR_SEPARATE_MSG_TAG) && (meta_header_sent[recv_status.MPI_SOURCE]) )
		/* we receive this message in the buffer for the sender process, directly after the meta header */
		MPI_Recv( meta_msg_i[recv_status.MPI_SOURCE] + 1, recv_msgcount, MPI_BYTE,
			  recv_status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_HOST, &recv_status);
	    else
		MPI_Recv(router_msg[0], recv_msgcount, MPI_BYTE, recv_status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_HOST, &recv_status);

#ifdef ROUTER_THREADS
	    pthread_mutex_unlock (&mpi_lock);
#endif
#ifndef WIN32
	    /* signal perfmeter server */
	    if (perfmon) WAKE_PERFSERVER;
#endif    
	    /* check type of message - command message or MPI message ? */
	    switch (recv_status.MPI_TAG) {
	    case MPIR_MPIMSG_TAG:
#if 0 /* CANCEL */
	        if( (recv_status.MPI_SOURCE == my_comm_host_rank) &&
		    (((Meta_Header *)router_msg[0])->msg.MPI.context_id == send_context) ) {
		    /* this is a cancel confirm message sent from the importing router
		     * to a process on another metahost
		     * because the importing router cannot access the meta header (it is added to the message later)
		     * we set the right mode here (this assumes that cancel confirm messages are the only
		     * messages that are sent from this proc to itself; if that doesn't hold true, it may lead
		     * to confusion
		     */
		    ((Meta_Header *)router_msg[0])->msg.MPI.mode = CANCEL_CONFIRM;
		}
#endif /* CANCEL */

		/* send the message to another host (router) */
		conn = get_conn_for_dest(((Meta_Header *)router_msg[0])->msg.MPI.dest_grank);
		
#ifdef META_DEBUG
		fprintf (stderr, "export_msgs(): send msg for [a%d] from [m%d], tag %d, MPI size %d\n",
			 ((Meta_Header *)router_msg[0])->msg.MPI.dest_grank,
			 ((Meta_Header *) router_msg[0])->msg.MPI.src_comm_lrank,
			 ((Meta_Header *) router_msg[0])->msg.MPI.tag,
			 ((Meta_Header *) router_msg[0])->msg.MPI.count);
		fflush (stderr);
#endif
	       	host_to_exch_int_struct(router_msg[0], sizeof(Meta_Header));


		conn_send_message((void *) router_msg[0], recv_msgcount, conn);
		/*	    show_global_time("router");*/

		break;

	    case MPIR_SEPARATE_MSG_TAG:
		conn = get_conn_for_dest( meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.dest_grank );

#ifdef META_DEBUG
		fprintf (stderr, "export msgs(): send msg for [a%d] from [m%d], tag %d, MPI size %d\n",
			 meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.dest_grank,
			 meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.src_comm_lrank,
			 meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.tag,
			 meta_msg_i[recv_status.MPI_SOURCE]->msg.MPI.count );
		fflush( stderr );
#endif
	       	host_to_exch_int_struct( meta_msg_i[recv_status.MPI_SOURCE], sizeof( Meta_Header ) );


#ifdef META_PIPELINE
		/* Send first message with information about msg size.
		   Receive all further pipeline msgs from this source and continue when
		   complete msg is transmitted. */
		conn_send_message_block( (void *) meta_msg_i[recv_status.MPI_SOURCE],
				    ((Meta_Header *)meta_msg_i[recv_status.MPI_SOURCE])->msg.MPI.count + sizeof( Meta_Header),
				    recv_msgcount + sizeof( Meta_Header ), conn );
		pipe_msg_size_left -= recv_msgcount;
		while ( pipe_msg_size_left != 0 ) {
		    MPI_Probe(recv_status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_ALL, &recv_status);
		    MPI_Get_count(&recv_status, MPI_BYTE, &recv_msgcount);
		    meta_msg_i[recv_status.MPI_SOURCE] = (Meta_Header * )adjustbuffer( (char *)(meta_msg_i[recv_status.MPI_SOURCE]),
										       meta_msg_i_size[recv_status.MPI_SOURCE],
										       recv_msgcount );
		    if ( meta_msg_i_size[recv_status.MPI_SOURCE] < recv_msgcount )
			meta_msg_i_size[recv_status.MPI_SOURCE] = recv_msgcount;
		    MPI_Recv( meta_msg_i[recv_status.MPI_SOURCE], recv_msgcount, MPI_BYTE,
			      recv_status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_ALL, &recv_status);
		    conn_send_message_block_append( (void *) meta_msg_i[recv_status.MPI_SOURCE], recv_msgcount, conn );
		    pipe_msg_size_left -= recv_msgcount;
		}
#else
		conn_send_message( (void *) meta_msg_i[recv_status.MPI_SOURCE], recv_msgcount + sizeof( Meta_Header ), conn );
#endif
		meta_header_sent[recv_status.MPI_SOURCE] = 0;
		break;
		
	    case MPIR_SEPARATE_META_HEADER_TAG:
		/* this is a short message containing the meta data for a nonblocking msg that comes later on, therefore
		   we must save the data here */
		memcpy( meta_msg_i[recv_status.MPI_SOURCE], router_msg[0], recv_msgcount );
		meta_header_sent[recv_status.MPI_SOURCE] = 1;
#ifdef META_PIPELINE
		pipe_msg_size_left = ((Meta_Header *)router_msg[0])->msg.MPI.count;
#endif
		break;

	    case MPIR_ROUTMSG_TAG:
		switch (((Meta_Header *)router_msg[0])->msg.Rout.command) {
		case FINALIZE:
		    mpi_app_status = MPI_APP_FINALIZE;
		    for (i = 0; i < nbr_threads; i++) 
			thrd_err = pthread_join (imp_threads[i], (void **)(thread_status + i));
		    /* free (router_msg[0]); */

		    RDEBUG2("%sr[%d]",RDEBUG_dbgprefix, MPIR_RouterConfig.myGlobalRouterRank);
		    RDEBUG(" got ROUTCMD_FINALIZE\n");
	    
		    break;
		default:
		    PRERROR2("%sr[%d]",RDEBUG_dbgprefix, MPIR_RouterConfig.myGlobalRouterRank);
		    PRERROR1(" unknown command: %d\n", ((int)((Meta_Header *)router_msg[0])->msg.Rout.command));
		     conn_close_connections();
		    ROUTER_ABORT;
		    break;
		}
		break;

	    default:
		PRERROR1("r[%d]", MPIR_RouterConfig.myGlobalRouterRank);
		PRERROR1(" got message with wrong tag: %d\n", recv_status.MPI_TAG);
		 conn_close_connections();
		ROUTER_ABORT;
	    }
	} /* if (flag) ... */ 
#ifdef ROUTER_THREADS
	/* unterminating while loop only exists in threaded version */
    }
#endif
    return 0;
}

/*
  This is the main router loop.
 */

int MPIR_Router(char *meta_config_file, int firstrank,
		int perfmeter, char *metahostname)
{
   /* common */
   int i;
   char mhname[256];
   /* MPI specific */
   MPI_Status      **snd_status;
   MPI_Request     **snd_request;
   struct MPIR_COMMUNICATOR *comm_host_ptr;

#ifdef WIN32
   WORD wVersionRequested;
   WSADATA wsaData;
#endif

   char            processor_name[MPI_MAX_PROCESSOR_NAME];
   int             namelen;
   int             procs_on_metahost;  /* number of aplication processes on this metahost */

   /* thread specific */
#ifdef ROUTER_THREADS
   pthread_mutexattr_t tcp_mutex_attr, mpi_mutex_attr;
   pthread_attr_t imp_threads_attr; 
   pthread_t perf_thread;
#else
   
#endif
   struct SThreadArgs_import *thread_args_import;
   struct SThreadArgs_export *thread_args_export;
   int thrd_id;
   int perf_conn;

   /* while(meta_barrier); */
   MPI_Get_processor_name(processor_name, &namelen);
   
   /* we calculate our router rank on our metahost (MPIR_RouterConfig.router_rank_on_metahost) */
   {
     Snode *node;
     int i, metahost_rank, router_rank;

     node = metahostlist[MPIR_meta_cfg.my_metahost_rank].nodeList;
     metahost_rank = 0;
     router_rank = 0;
     /* loop over all nodes on our metahost */
     while( node ) {
       for( i = 0; i < node->numRouters; i++ ) {
	 if( metahost_rank == MPIR_meta_cfg.my_rank_on_metahost )
	   MPIR_RouterConfig.router_rank_on_metahost = router_rank;
	 metahost_rank++;
	 router_rank++;
       }
       metahost_rank += node->np+node->npExtraProcs;
       node = node->next;
     }
   }



#ifdef WIN32
   wVersionRequested = MAKEWORD( 2, 2 );
   if( WSAStartup( wVersionRequested, &wsaData ) != 0 ) {
       i = WSAGetLastError();
       PRERROR1("WSAStartup() failed. Error %d\n",i);
       ROUTER_ABORT_INIT;
   }
#endif
   
   strcpy(RDEBUG_dbgprefix, "[");
   strcpy(mhname, MPIR_meta_cfg.my_metahostname);
   strcat (RDEBUG_dbgprefix, mhname);
   strcat(RDEBUG_dbgprefix, "|" );

   strcat (RDEBUG_dbgprefix, MPIR_meta_cfg.nodeName);
   strcat(RDEBUG_dbgprefix, "|r" );
   sprintf(&(RDEBUG_dbgprefix[strlen(RDEBUG_dbgprefix)]) , "%d|%s|%d",  MPIR_RouterConfig.router_rank_on_metahost, MPIR_process_name, getpid());

   strcat(RDEBUG_dbgprefix, "]" );
   
   PRVERBOSE("Initializing....\n");
   /* calculation of MPIR_RouterConfig.myGlobalRouterRank */
   {
     int h_id,rank;

     rank = 0;
     for( h_id = 0; h_id < MPIR_meta_cfg.nbr_metahosts; h_id++ ) {
	 if( MPIR_meta_cfg.my_metahost_rank == h_id )
	     MPIR_RouterConfig.myGlobalRouterRank=rank+MPIR_RouterConfig.router_rank_on_metahost;
	 rank += MPIR_meta_cfg.nrp_metahost[h_id];
     }
   }

   /* calculation of MPIR_RouterConfig.otherhostid */
   {
     int router_rank_on_metahost;

     for( router_rank_on_metahost = 0; router_rank_on_metahost < rh_getNumRouters(); router_rank_on_metahost++ ) {
       if( routerlist[router_rank_on_metahost].metahostrank == MPIR_meta_cfg.my_rank_on_metahost )
	 MPIR_RouterConfig.otherhostid = routerlist[router_rank_on_metahost].host;
     }
   }

   if ( MPIR_RouterConfig.isHetero)
       MPID_router_byte_order = get_byte_order();  /* set byte order on host */
   
   /* intialize connections */
   if (conn_init_connections(MPIR_RouterConfig.split_size) == -1) {
       RERROR("error in init_connections\n");
       ROUTER_ABORT_INIT;
   }
   
   if (open_connections() < 0) {
       RERROR("error in open_connections\n");
       ROUTER_ABORT_INIT;
   }
   for (i = 0; i <  conn_getnumconn(); i++) 
       sync_connection(i);

#ifndef WIN32  
   /* init the perfmeter server */
   perfmon = perfmeter;
   if (perfmon) init_perfmeter();
#endif   
   /* let the show begin */
   MPI_Barrier(MPI_COMM_HOST);
   mpi_app_status = MPI_APP_RUNNING;

   /* initialize arrays for non-blocking, split messages */
   procs_on_metahost = MPIR_meta_cfg.np_metahost[MPIR_meta_cfg.my_metahost_rank]
     + MPIR_meta_cfg.nrp_metahost[MPIR_meta_cfg.my_metahost_rank];
   meta_msg_i = (Meta_Header **) malloc( procs_on_metahost * sizeof( Meta_Header * ) );
   meta_msg_i_size = (int *) malloc( procs_on_metahost * sizeof( int ) );
   meta_header_sent = (int *) malloc( procs_on_metahost * sizeof( int ) );
   for( i = 0; i < procs_on_metahost; i++ ) {
     if( !(MPIR_meta_cfg.isRouter[i]) ) {
       meta_msg_i[i] = (Meta_Header *) malloc( INIT_ROUTER_BUFFER_SIZE * sizeof(char) );
       meta_msg_i_size[i] = INIT_ROUTER_BUFFER_SIZE * sizeof(char);
       meta_header_sent[i] = 0;
     }
   }

   
   /* import_msgs: initialize thread-specific data and create the threads */
   nbr_threads = THREADS_PER_CONNECTION *  conn_getnumconn();  /* XXX until now, only 1 thread per conn ! */
#ifdef ROUTER_THREADS
   pthread_mutexattr_init (&tcp_mutex_attr);
   pthread_mutexattr_init (&mpi_mutex_attr);
   pthread_mutex_init (&tcp_lock, &tcp_mutex_attr);
   pthread_mutex_init (&mpi_lock, &mpi_mutex_attr);
   pthread_attr_init (&imp_threads_attr);
#endif
   snd_status = (MPI_Status **) calloc (nbr_threads, sizeof (MPI_Status *));
   snd_request = (MPI_Request **) calloc (nbr_threads, sizeof (MPI_Request *));
   thread_status = (int *) calloc (nbr_threads, sizeof (int));
   imp_threads = (pthread_t *) calloc (nbr_threads, sizeof (pthread_t));
   thread_args_import = (struct SThreadArgs_import *) calloc(nbr_threads, sizeof (struct SThreadArgs_import));
   
   for (thrd_id = 0; thrd_id < nbr_threads; thrd_id++) {
      snd_status[thrd_id] = (MPI_Status *) calloc (MPIR_RouterConfig.isend_num, sizeof (MPI_Status));
      thread_args_import[thrd_id].snd_status = snd_status[thrd_id];
      snd_request[thrd_id] = (MPI_Request *) calloc (MPIR_RouterConfig.isend_num, sizeof (MPI_Request));
      thread_args_import[thrd_id].snd_request = snd_request[thrd_id];
      
      thread_args_import[thrd_id].thread_id  = thrd_id;
      thread_args_import[thrd_id].connection = thrd_id;
      
      Qinit (&thread_args_import[thrd_id].availQ, MPIR_RouterConfig.isend_num, 1);
      Qinit (&thread_args_import[thrd_id].pendingQ, MPIR_RouterConfig.isend_num, 0);

      if( !(thread_args_import[thrd_id].router_msg = (char **)malloc( MPIR_RouterConfig.isend_num * sizeof(char *))) ) {
	  PRERROR( "Could not allocate enough local memory" );
	  ROUTER_ABORT;
      }	
      thread_args_import[thrd_id].bufsize = (int *)malloc( MPIR_RouterConfig.isend_num * sizeof(int) );
      for (i = 0; i < MPIR_RouterConfig.isend_num; i++) {
	  if( !(thread_args_import[thrd_id].router_msg[i]  = (char *)malloc( INIT_ROUTER_BUFFER_SIZE * sizeof(char))) ) {
	      PRERROR( "Could not allocate enough local memory" );
	      ROUTER_ABORT;
	  }
	  thread_args_import[thrd_id].bufsize[i]   = INIT_ROUTER_BUFFER_SIZE;
      }

#ifdef ROUTER_THREADS      
      pthread_create (&imp_threads[thrd_id], &imp_threads_attr, import_msgs, 
                      (void *)&thread_args_import[thrd_id]);
#endif
   }	


   /* export_msgs: initialize thread-specific data and create the threads */

   /* the export is done single-threaded (until now) */
   /* only 1 thread_buffer is needed until export_msgs is multi-threaded */
   thread_args_export = (struct SThreadArgs_export *) calloc(1, sizeof (struct SThreadArgs_export));
   for (thrd_id = 0; thrd_id < 1; thrd_id++) {
       thread_args_export[thrd_id].thread_id  = thrd_id;
       thread_args_export[thrd_id].connection = thrd_id;

       /* only 1 buffer is needed until export_msgs is multi-threaded */
       if( !(thread_args_export[thrd_id].router_msg = (char **)malloc( 1 * sizeof(char *))) ) {
	   PRERROR( "Could not allocate enough local memory" );
	   ROUTER_ABORT;
       }
       thread_args_export[thrd_id].bufsize = (int *)malloc( 1 * sizeof(int) );
       for (i = 0; i < 1; i++) {
	   if( !(thread_args_export[thrd_id].router_msg[i]  = (char *)malloc( INIT_ROUTER_BUFFER_SIZE * sizeof(char))) ) {
	       PRERROR( "Could not allocate enough local memory" );
	       ROUTER_ABORT;
	   }
	   thread_args_export[thrd_id].bufsize[i]   = INIT_ROUTER_BUFFER_SIZE;
       }

       comm_host_ptr = MPIR_GET_COMM_PTR( MPI_COMM_HOST );
       thread_args_export[thrd_id].my_comm_host_rank = comm_host_ptr->local_rank;
       thread_args_export[thrd_id].send_context = comm_host_ptr->send_context;
       

#ifdef ROUTER_THREADS   
       /* the export is done single-threaded (until now) */
       export_msgs ( (void*)&thread_args_export[0] );
#endif
   }

#ifndef ROUTER_THREADS
   /* NON_THREADED Version!
      import_msgs and export_msgs are called alternately from an endless loop.
      Thread variables are used for compatibility reasons.
   */
   while (mpi_app_status == MPI_APP_RUNNING) {
       for (thrd_id = 0; thrd_id < nbr_threads; thrd_id++) {
	   import_msgs((void *)&thread_args_import[thrd_id]);
       }
       export_msgs( (void*)&thread_args_export[0] );
   }
#endif

   
   /* synchronize connected routers before shutting down */
   for (i = 0; i <  conn_getnumconn(); i++) {
      sync_connection(i);
   }
   
   for( i = 0; i < procs_on_metahost; i++ ) {
     if( !(MPIR_meta_cfg.isRouter[i] ) )
	 free( meta_msg_i[i] );
   }
   free( meta_msg_i );
   free( meta_msg_i_size );
   free( meta_header_sent );

   MPI_Finalize();
   
#ifndef WIN32   
   /* tell the performance server to exit */
   if (perfmon) terminate_perfmeter();
#endif   
   conn_close_connections();
   conn_remove_connections();
#ifdef WIN32  
   RDEBUG("WSACleanup()\n");
   WSACleanup();
#endif

   for (thrd_id = 0; thrd_id < nbr_threads; thrd_id++) {
       free (snd_status[thrd_id]);
       free (snd_request[thrd_id]);
       
       Qdestroy (&thread_args_import[thrd_id].availQ);
       Qdestroy (&thread_args_import[thrd_id].pendingQ);

       for (i = 0; i < MPIR_RouterConfig.isend_num; i++)
	   free( thread_args_import[thrd_id].router_msg[i] );
       free( thread_args_import[thrd_id].router_msg );
       free( thread_args_import[thrd_id].bufsize );
   }
   
   free (snd_request);
   free (snd_status);
   free (thread_args_import);
   free (thread_status);
   free (imp_threads);

   for (thrd_id = 0; thrd_id < 1; thrd_id++) {
       for (i = 0; i < 1; i++)
	   free( thread_args_export[thrd_id].router_msg[i] );
       free( thread_args_export[thrd_id].router_msg );
       free( thread_args_export[thrd_id].bufsize );
   }
   free (thread_args_export);
   
   return MPI_SUCCESS;
}

/* end of mpi_router.c */


