/* $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 * 
 * this module contains functions to establish the necessary network connections
 * for the routing process
 */


#include "connection.h"
#define CONN_THREADS

static int num_conn;                 /* number of connections */
static struct SConnection *connlist; /* array with connections */

static int globalSplitSize;               /* threshold value to split messages */

int *send_barrier_args;
int *recv_barrier_args;

#define MAX_ONE_COPY_SIZE 512
char *tmp_sendbuf;    /* sendbuffer for smaller messages */


                      /* This struct is an envelope for sending messages over network.
The data is stored in host order */
struct SBuf {
  unsigned short int    magic; /* magic number for identification of envelope */
  int          size;           /* if we send (receive) a message of size < split_size, 
				  size = total_msgsize; otherwise size indicates
				  the size of the part of the massage to send (receive) on
				  particular socket: 
				  size = total_msgsize / nbr_of_sockets */
  int          total_msgsize;  /* size of the message (size of reference 
				  data without envelope */
};

struct SConnection;


/* one send thread and one recv thread for each socket */
struct SSocketThread {
   struct SSocket  *socket;    /* pointer to corresponding logical socket */
   pthread_t       thread;     
   int             thread_id;  /* internal ID */
   size_t          buf_size;   /* buffersize and buffer for data to transmit */
                               char           *net_buf;    /* stores raw data for sending or receiving 
                               on the corresponding logical socket */
};


struct SConnThread {
   int             use_sockets;     /* nbr of sockets to use for the current transfer */
   size_t          total_msgsize;   
   size_t          buffersize;
   struct SBuf     header_buf;      /*  envelope for sending messages over network */
   
   /* thread synchronization */
   pbarrier_t      conn_barrier;
   pthread_cond_t  conn_cv;
   pthread_mutex_t conn_lock;
   
   /* thread control */
   pthread_mutex_t action_lock;
   volatile conn_thread_action conn_action;
};

/* socket-relevant data */
struct SSocket {
   int             localsocket;
   int             remotesocket;

   struct sockaddr *loc_addr;
   struct sockaddr *rem_addr;

   
   /* QOS string to be converted by text2qos later */
   char qos[256];                 
   /* max SDU sizes for send() and recv() operations on this socket 
	relevant for ATM sockets only, ignored by TCP/IP sockets */
   unsigned long rcvMaxSduSize;
   unsigned long sndMaxSduSize;

   struct SSocket *next;
   struct SConnection *conn;
   struct SSocketThread *send_thread, *recv_thread;
};


/*
* struct SConnection contains all necessary information of a logical
* connection to another router
*/
struct SConnection {
   int             conn_nbr;       /* index of this connection */
   
   int             nbr_sockets;    /* number of send/recv-sockets available in this connection */
   long            splitSize; 
   struct SSocket  sockets;
   
   unsigned short  sa_family;      /* address family, AF_INET, PF_ATMSVC, PF_ATMPVC */
   int             addr_len;       /* length of loc_addr and rem_addr in SSocket */
   
   int             serv;           /* are we SERVER or CLIENT ? */
   int             state;          /* are we RUNNING or shall we FINALIZE ? */
   int             header_is_read; /* only used by receiving threads */
  int timeout; /* timeout for connect - retry until timeout */
   /* synchronization and state data for the send- and recv-threads */
   struct SConnThread send, recv;
   int (*read)(void *buffer, unsigned size, int fd, unsigned long maxSDUsize);
   int (*write)(void *buffer, unsigned size, int fd, unsigned long maxSDUsize);
   int (*establish_socket_connection)(int *localsocket_fd, struct sockaddr *loc_addr,
				      int *remotesocket_fd, struct sockaddr *rem_addr,
				      char *qos, unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize, 
				      int max_nconn, int serv, int timeout);
   int (*add_socket_pair)(int *localsocket, int *remotesocket, 
      unsigned short sa_family, int serv);
};


/*int  select_connection    (int conn, int timeout);
int  check_message        (int conn, int mode);
int  select_all           (int timeout);
*/
int conn_getnumconn()
{
   return num_conn;
}

int get_conn_type(int conn)
{
   if (conn >= num_conn) {
      RERROR2("%sget_conn_type: non-existent connection handle: %d\n", RDEBUG_dbgprefix, conn);
      return -1;
   }
   return connlist[conn].serv;
}

/* No changes */
int conn_init_connections(int splitSize)
{
   int             i;
   globalSplitSize=splitSize;
   /* initialize counter for connlist */
   num_conn = 0;
   if ((connlist = (struct SConnection *) malloc(MAX_CONN_COUNT * sizeof(struct SConnection))) == NULL) {
      RDEBUG("conn_init_connections: malloc failed\n");
      return -1;
   }
   connlist->sockets.next = NULL;
   if ((send_barrier_args = (int *) malloc(MAX_CONN_COUNT * sizeof(int))) == NULL) {
      RDEBUG("conn_init_connections: malloc failed\n");
      return -1;
   }
   if ((recv_barrier_args = (int *) malloc(MAX_CONN_COUNT * sizeof(int))) == NULL) {
      RDEBUG("conn_init_connections: malloc failed\n");
      return -1;
   }
   if ((tmp_sendbuf = (char *) malloc(MAX_ONE_COPY_SIZE)) == NULL) {
      RDEBUG("conn_init_connections: malloc failed\n");
      return -1;
   }
   for (i = 0; i < MAX_CONN_COUNT; i++) {
      send_barrier_args[i] = i;
      recv_barrier_args[i] = i;
   }
   return 1;
}



/*
* the threads are running this function in which they send  via their individual socket
*/
void *send_threads(void *send_thread_args)
{
   struct SConnection *my_conn;
   struct SSocket *my_sle;
   struct SSocketThread *my_thrd = (struct SSocketThread *) send_thread_args;
   
   my_sle  = my_thrd->socket; 
   my_conn = my_sle->conn;
   pbarrier_wait(&(my_conn->send.conn_barrier));
   
   while (my_conn->send.conn_action != QUIT) {
      /* wait for wake-up */
      pthread_mutex_lock(&(my_conn->send.action_lock));
      while (my_conn->send.conn_action == WAIT) {
         pthread_cond_wait(&(my_conn->send.conn_cv), &(my_conn->send.action_lock));
      }
      
      switch (my_conn->send.conn_action) {
      case WRITE:
         pthread_mutex_unlock(&(my_conn->send.action_lock));
         /* write data to socket */
         if (my_thrd->thread_id < my_conn->send.use_sockets) {
	     if (my_thrd->thread_id == 0) {
		 if (my_thrd->buf_size+sizeof(struct SBuf) <= MAX_ONE_COPY_SIZE) {
		     /* copy header and buffer to avoid 2 write calls */
		     memcpy(tmp_sendbuf, &my_conn->send.header_buf, sizeof(struct SBuf));
		     memcpy((struct SBuf*)tmp_sendbuf + 1, my_thrd->net_buf, my_thrd->buf_size);
		     my_conn->write(tmp_sendbuf, my_thrd->buf_size+sizeof(struct SBuf), 
				    my_sle->remotesocket, my_sle->sndMaxSduSize);
		 } else   { /* header */
		     my_conn->write((void *) &(my_conn->send.header_buf), sizeof(struct SBuf), my_sle->remotesocket, my_sle->sndMaxSduSize);
		     my_conn->write((void *) my_thrd->net_buf, my_thrd->buf_size, my_sle->remotesocket, my_sle->sndMaxSduSize);
		 }
	     } else {
		 /* data */
		 my_conn->write((void *) my_thrd->net_buf, my_thrd->buf_size, my_sle->remotesocket,my_sle->sndMaxSduSize);
	     }
         }
         pbarrier_wait(&(my_conn->send.conn_barrier));
         break;
      case READ:
         pthread_mutex_unlock(&(my_conn->send.action_lock));
         RERROR2("%ssend_threads: illegal action READ %d: ", RDEBUG_dbgprefix, my_conn->conn_nbr);
         perror("");
         exit(-1);
         break;
      case QUIT:
         pthread_mutex_unlock(&(my_conn->send.action_lock));
         if (my_conn->serv == CONN_CLIENT)
            close(my_sle->remotesocket);
         break;
      case RESYNC:
         pthread_mutex_unlock(&(my_conn->send.action_lock));
         /* just sync with the other threads (needed if a
         socket(thread) is added) */
         pbarrier_wait(&(my_conn->send.conn_barrier));
         break;
      case ADDSOCK:
         pthread_mutex_unlock(&(my_conn->send.action_lock));
         pbarrier_wait_and_inc(&(my_conn->send.conn_barrier));
         break;
      case WAIT:
         pthread_mutex_unlock(&(my_conn->send.action_lock));
         RERROR2("%ssend_threads: illegal state WAIT for connection %d: ",
            RDEBUG_dbgprefix, my_conn->conn_nbr);
         perror("");
         exit(-1);
      default:
         pthread_mutex_unlock(&(my_conn->send.action_lock));
         RERROR2("%ssend_threads: undefined action for connection %d: ",
            RDEBUG_dbgprefix, my_conn->conn_nbr);
         perror("");
         exit(-1);
      }
   }
   pthread_exit(NULL);
   return 0;
}

/*
* the threads are running this function in which they send and receive via
* their individual socket. recv_threads() is waked up by recv_message() if 
* there is data to receive on corresponding logical socket
*/
void *recv_threads(void *recv_thread_args)
{
   struct SConnection *my_conn;
   struct SSocket *my_sle;
   struct SSocketThread *my_thrd = (struct SSocketThread *) recv_thread_args;
   
   my_sle  = my_thrd->socket; 
   my_conn = my_sle->conn;
   pbarrier_wait(&(my_conn->recv.conn_barrier));
   
   while (my_conn->recv.conn_action != QUIT) {
      /* wait for wake-up */
      pthread_mutex_lock(&(my_conn->recv.action_lock));
      while (my_conn->recv.conn_action == WAIT) {
         pthread_cond_wait(&(my_conn->recv.conn_cv), &(my_conn->recv.action_lock));
      }
      
      switch (my_conn->recv.conn_action) {
      case WRITE:
         pthread_mutex_unlock(&(my_conn->recv.action_lock));
         RERROR2("%srecv_threads: illegal action WRITE %d: ", RDEBUG_dbgprefix, my_conn->conn_nbr);
         perror("");
         exit(-1);
         break;
      case READ:
         pthread_mutex_unlock(&(my_conn->recv.action_lock));
         if (my_thrd->thread_id < my_conn->recv.use_sockets) {
            /* read my part of the message */
            my_conn->read((void *) my_thrd->net_buf, my_thrd->buf_size, my_sle->remotesocket, my_sle->rcvMaxSduSize);
         }
         pbarrier_wait(&(my_conn->recv.conn_barrier));
         break;
      case QUIT:
         pthread_mutex_unlock(&(my_conn->recv.action_lock));
         if (my_conn->serv == CONN_CLIENT)
            close(my_sle->remotesocket);
         break;
      case RESYNC:
         pthread_mutex_unlock(&(my_conn->recv.action_lock));
         /* just sync with the other threads (needed if a
         socket(thread) is added) */
         pbarrier_wait(&(my_conn->recv.conn_barrier));
         break;
      case ADDSOCK:
         pthread_mutex_unlock(&(my_conn->recv.action_lock));
         pbarrier_wait_and_inc(&(my_conn->recv.conn_barrier));
         break;
      case WAIT:
         pthread_mutex_unlock(&(my_conn->recv.action_lock));
         RERROR2("%srecv_threads: illegal state WAIT for connection %d: ",
            RDEBUG_dbgprefix, my_conn->conn_nbr);
         perror("");
         exit(-1);
      default:
         pthread_mutex_unlock(&(my_conn->recv.action_lock));
         RERROR2("%srecv_threads: undefined action for connection %d: ",
            RDEBUG_dbgprefix, my_conn->conn_nbr);
         perror("");
         exit(-1);
      }
   }
   pthread_exit(NULL);
   return 0;
}


/* adds a socket connection to the logical connection */
int conn_add_socket(int conn, struct sockaddr *local_addr, 
               struct sockaddr *remote_addr, char *qos)
{
   struct SConnection *C;
   struct SSocket *sle;
   struct SSocketThread *send_thread, *recv_thread;
   int             i, err;
   pthread_attr_t send_threads_attr, recv_threads_attr;
   
   C = &connlist[conn];
   /* check parameters and list integrity */
   if (conn >= num_conn) {
      RERROR2("%sconn_add_socket: non-existent connection handle: %d\n", RDEBUG_dbgprefix, conn);
      return -1;
   }  
   sle = &(C->sockets);
   i = 1;
   while (sle->next != NULL) {
      i++;
      sle = sle->next;
   }
   /* create socket list entries and the corresponding socket */
   if( C->nbr_sockets > 0 ) {
       if (i != C->nbr_sockets) {
	   RERROR1("%sconn_add_socket: internal error: corrupted socket list\n",RDEBUG_dbgprefix);
	   exit(-1);
       }
       sle->next = (struct SSocket *) malloc(sizeof(struct SSocket));
       sle = sle->next;
       sle->next = NULL;
   }
   sle->conn = &connlist[conn];
   
   sle->loc_addr = malloc(C->addr_len);
   sle->rem_addr = malloc(C->addr_len);
   memcpy(sle->loc_addr, local_addr, C->addr_len);
   memcpy(sle->rem_addr, remote_addr, C->addr_len);
   
   if(qos)
      strcpy(sle->qos, qos);
   
   err = C->add_socket_pair(&sle->localsocket, &sle->remotesocket, C->sa_family, C->serv);
   
   if (err < 0) {
      RERROR2("%sconn_add_socket: cannot add socket pair for conn %d\n", RDEBUG_dbgprefix, num_conn);
      exit(-1);
   }
   C->nbr_sockets++;
   /* create the threads which will handle this socket */
   /* send thread */
   send_thread = (struct SSocketThread *) malloc(sizeof(struct SSocketThread));
   send_thread->thread_id = C->nbr_sockets - 1;
   send_thread->socket = sle;
   sle->send_thread = send_thread;
   if( C->nbr_sockets > 1 ) {/* the are more sockets already attached to this logical connection,
      do some synchronization */
      pthread_mutex_lock(&(C->send.action_lock));
      C->send.conn_action = ADDSOCK;
      pthread_mutex_unlock(&(C->send.action_lock));
      pthread_cond_broadcast(&C->send.conn_cv);
      pbarrier_wait_and_inc(&(C->send.conn_barrier));
   }
   pthread_attr_init(&send_threads_attr);
   pthread_create(&send_thread->thread, &send_threads_attr, send_threads, (void *)send_thread);
   if( C->nbr_sockets > 1 ) {/* the are more sockets already attached to this logical connection,
      do some synchronization */
      pthread_mutex_lock(&(C->send.action_lock));
      C->send.conn_action = RESYNC;
      pthread_mutex_unlock(&(C->send.action_lock));
      pthread_cond_broadcast(&C->send.conn_cv);
   }
   pbarrier_wait(&(C->send.conn_barrier));
   /* recv thread */
   recv_thread = (struct SSocketThread *) malloc(sizeof(struct SSocketThread));
   recv_thread->thread_id = C->nbr_sockets - 1;
   recv_thread->socket = sle;
   sle->recv_thread = recv_thread;
   if( C->nbr_sockets > 1 ) {/* the are more sockets already attached to this logical connection,
      do some synchronization */
      pthread_mutex_lock(&(C->recv.action_lock));
      C->recv.conn_action = ADDSOCK;
      pthread_mutex_unlock(&(C->recv.action_lock));
      pthread_cond_broadcast(&C->recv.conn_cv);
      pbarrier_wait_and_inc(&(C->recv.conn_barrier));
   }
   pthread_attr_init(&recv_threads_attr);
   pthread_create(&recv_thread->thread, &recv_threads_attr, recv_threads, (void *)recv_thread);
   if( C->nbr_sockets > 1 ) {/* the are more sockets already attached to this logical connection,
      do some synchronization */
      pthread_mutex_lock(&(C->recv.action_lock));
      C->recv.conn_action = RESYNC;
      pthread_mutex_unlock(&(C->recv.action_lock));
      pthread_cond_broadcast(&C->recv.conn_cv);
   }
   pbarrier_wait(&(C->recv.conn_barrier));
   return 1;
}  


void send_setwait(void *arg)
{
   pthread_mutex_lock(&(connlist[*(int *) arg].send.action_lock));
   connlist[*(int *) arg].send.conn_action = WAIT;
   pthread_mutex_unlock(&(connlist[*(int *) arg].send.action_lock));
   return;
}

void recv_setwait(void *arg)
{
   pthread_mutex_lock(&(connlist[*(int *) arg].recv.action_lock));
   connlist[*(int *) arg].recv.conn_action = WAIT;
   pthread_mutex_unlock(&(connlist[*(int *) arg].recv.action_lock));
   return;
}

int conn_add_connection(struct sockaddr *local_addr, 
                   struct sockaddr *remote_addr, 
                   unsigned short  sa_family, /* address family, AF_INET, PF_ATMSVC, PF_ATMPVC */
                   char *qos, int serv, int timeout)
{
   struct SConnection *C;
   int new_conn;
   
   new_conn = num_conn;
   
   if (new_conn == MAX_CONN_COUNT) {
      RERROR1("%conn_sadd_connection: too many connections\n",RDEBUG_dbgprefix);
      return -1;
   }
   
   C = &connlist[new_conn];
   C->splitSize=globalSplitSize;
   C->timeout=timeout;
   /* 
   *  multi-threading environment 
   */
   /* sending threads */
   pbarrier_init(&(C->send.conn_barrier), 2, send_setwait, 
      (void *) &(send_barrier_args[new_conn]));
   pthread_mutex_init(&C->send.conn_lock, NULL);
   pthread_mutex_init(&C->send.action_lock, NULL);
   pthread_cond_init(&C->send.conn_cv, NULL);
   C->send.conn_action = WAIT;
   /* receiving threads */
   pbarrier_init(&(C->recv.conn_barrier), 2, recv_setwait, 
      (void *) &(recv_barrier_args[new_conn]));
   pthread_mutex_init(&C->recv.conn_lock, NULL);
   pthread_mutex_init(&C->recv.action_lock, NULL);
   pthread_cond_init(&C->recv.conn_cv, NULL);
   C->recv.conn_action = WAIT;
   
   /* socket environment */
   C->nbr_sockets = 0;
   C->serv = serv;
   C->header_is_read = 0;
   C->state = CONNSTATE_DOWN;
   C->sa_family = sa_family;
   
   switch(sa_family) {
#ifdef META_ATM
   case (PF_ATMSVC): 
      C->write = atm_write;
      C->read  = atm_read;
      C->establish_socket_connection = atm_establish_svc_sock_conn;
      C->conn_add_socket_pair = atm_conn_add_socket_pair;
      C->addr_len = sizeof(struct sockaddr_atmsvc);
      break;
   case (PF_ATMPVC): 
      C->write = atm_write;
      C->read  = atm_read;
      C->establish_socket_connection = atm_establish_pvc_sock_conn;
      C->conn_add_socket_pair = atm_conn_add_socket_pair;
      C->addr_len = sizeof(ATM_PVC_PARAMS);
      break;
#endif
   default: 
      C->write = tcp_write;
      C->read  = tcp_read;
      C->establish_socket_connection = tcp_establish_sock_conn;
      C->add_socket_pair = tcp_add_socket_pair;
      C->addr_len = sizeof(struct sockaddr_in);
      break;
   }
   
   num_conn++;
   conn_add_socket(new_conn, local_addr, remote_addr, qos);
   return new_conn;
}  


/*******************************************************************************
* conn_establish_connection() establishes network connections on each socket of the
* given logical connection indicated by conn. 
* The main tasks of conn_establish_connection() are 
* - consistency check of logical connection
* - call of appropriate function to establish network connections depending on 
* their type: ATM, TCP/IP, ...
*******************************************************************************/

int conn_establish_connection(int conn) {
   struct SConnection *C;
   struct SSocket *sle;
   int err = 0;
   C = &connlist[conn];
   sle = &C->sockets;
   
   /* consistency check */
    if (conn >= num_conn) {
	RERROR2("%sconn_establish_connection: non-existent connection handle: %d\n", RDEBUG_dbgprefix, conn);
	return -1;
    }
    if (connlist[conn].state == CONNSTATE_UP) {
	RERROR1("%sconn_establish_connection: connection already established\n",RDEBUG_dbgprefix);
	return -1;
    }
    if ((connlist[conn].serv == CONN_CLIENT) && (connlist[conn].state != CONNSTATE_DOWN)) {
	RERROR1("%sconn_establish_connection: connection not down\n",RDEBUG_dbgprefix);
	return -1;
    }
    RDEBUG3("%sestablishing connection %d with timeout %ds...\n", RDEBUG_dbgprefix, conn, C->timeout);
   
    while( (sle != NULL) && 
	   ((err = C->establish_socket_connection(
						  &(sle->localsocket), sle->loc_addr, 
						  &(sle->remotesocket), sle->rem_addr,
						  sle->qos,
						  &sle->rcvMaxSduSize, &sle->sndMaxSduSize,
						  MAX_CONN_COUNT, C->serv, C->timeout)) == 0))
	sle = sle->next;
   
    return err;
}


/*******************************************************************************
* close_connection() terminates all threads which for their part close every  
* connection in connlist, if client.                                          
*******************************************************************************/
void conn_close_connections()
{
   struct SSocket *sle;
   struct SConnection *C;
   int             conn;
   char            s[255];
   void           *ret;
   
   /* iterate through connlist */
   for (conn = 0; conn < num_conn; conn++) {
      sprintf(s, "closing connection %d\n", conn);
      RDEBUG(s);
      
      C = &connlist[conn];
      /* terminate threads which close their socket */
      pthread_mutex_lock(&(C->send.action_lock));
      C->send.conn_action = QUIT;
      pthread_mutex_unlock(&(C->send.action_lock));
      pthread_cond_broadcast(&C->send.conn_cv);
      
      pthread_mutex_lock(&(C->recv.action_lock));
      C->recv.conn_action = QUIT;
      pthread_mutex_unlock(&(C->recv.action_lock));
      pthread_cond_broadcast(&C->recv.conn_cv);
      
      sle = &(C->sockets);
      while (sle != NULL) {
         pthread_join(sle->send_thread->thread, &ret);
         pthread_join(sle->recv_thread->thread, &ret);
         sle = sle->next;
      }
      
      C->state = CONNSTATE_DOWN;
   }
   RDEBUG("conn_close_connections(): connections closed \n");
}

/******************************************************************************
* = O: OK
* < 0: Error 
* conn_send_message() sends data of size bufsize pointed to by sendbuf on specified 
* connection conn.
* One of the main tasks of send_message() is to splite sendbuf, if it exceeds 
* split_size, and to invoke send_thread's to send splitted parts of the 
* sendbuf over network.
* If sendbuf < split_size, send_message() doesn't splite sendbuf. It 
* straightly sends the whole sendbuf without using of socket threads. sendbuf 
* is then sent on the first remote socket of the given connection.
*
* send_message_block() and send_message_block_append() 
*******************************************************************************/

int conn_send_message_block(void *sendbuf, size_t total_bufsize, size_t act_bufsize, int conn)
{
    struct SSocket *sle;
    struct SConnection *C;
    int             thrd;
    int             socket_bufsize;
   
    FN_IN_DEBUG("send_message");
   
    if (conn >= num_conn) {
	RERROR2("%ssend_message: non-existent conn handle: %d\n", RDEBUG_dbgprefix, conn);
	return -1;
    }
    C = &connlist[conn];
   
    /* locking is done on connection level -> might be done on thread
     * level in the future to enable overlapping message transmission
     * (this will require more advanced buffer management)
     */
#ifdef CONN_THREADS
    pthread_mutex_lock(&(C->send.conn_lock));
#endif   
    C->send.header_buf.magic = htons(MAGIC_HEADER);
    C->send.header_buf.total_msgsize = htonl(total_bufsize);
    sle = &(C->sockets);
    /* workaround for performance loss when transmitting small messages
       (as observed under Solarisx86): even when only one thread is 
       scheduled to tranmit the data (because bufsize < split_size),
       the synchronisation of the other threads seems to dramatically bring 
       down the effective bandwith */
    if (total_bufsize <= C->splitSize) {
	C->send.header_buf.size = htonl(total_bufsize);
	if (total_bufsize + sizeof(struct SBuf) <= MAX_ONE_COPY_SIZE) {
	    /* copy header and buffer to avoid 2 write calls causing extended latency */
	    memcpy(tmp_sendbuf, (void *)&(C->send.header_buf), sizeof(struct SBuf));
	    memcpy((struct SBuf*)tmp_sendbuf + 1, sendbuf, total_bufsize);
	    if( C->write((void *)tmp_sendbuf, total_bufsize + sizeof(struct SBuf), 
			 sle->remotesocket, sle->sndMaxSduSize) < 0 )
		return -1;
	} else {
	    /* header */
	    if( C->write((void *)&(C->send.header_buf), sizeof(struct SBuf), 
			 sle->remotesocket, sle->sndMaxSduSize) < 0 )
		return -1;
	    /* data */
	    if( C->write((void *)sendbuf, act_bufsize, sle->remotesocket,
			 sle->sndMaxSduSize) != act_bufsize )
		return -1;
	}
    } else {
	/* nbr of sockets to use can be between 1 and nbr_sockets */
	C->send.use_sockets = act_bufsize / C->splitSize + (act_bufsize % C->splitSize != 0);
	if (C->send.use_sockets > C->nbr_sockets)
	    C->send.use_sockets = C->nbr_sockets;
	if (C->send.use_sockets == 0)
	    C->send.use_sockets = 1;
	socket_bufsize = act_bufsize / C->send.use_sockets;
      
	/* distribute message buffer on the threads
	 * 
	 * we directly use the send buffer given from above - this way we save
	 * one memcpy, but we have to submit two write's instead of one.
	 * A matter of optimization...
	 */	    
	for (thrd = 0; thrd < C->send.use_sockets; thrd++) {
	    sle->send_thread->net_buf = ((char *) sendbuf) + thrd * socket_bufsize;
	    sle->send_thread->buf_size = socket_bufsize;
	    if (thrd == C->send.use_sockets - 1)
		sle->send_thread->buf_size += act_bufsize % C->send.use_sockets;
         
	    sle = sle->next;
	}	
	C->send.header_buf.size = htonl(act_bufsize/C->send.use_sockets);
	/* wake up the threads and let them send the message */
#ifdef CONN_THREADS
	pthread_mutex_lock(&(C->send.action_lock));
#endif
	C->send.conn_action = WRITE;
#ifdef CONN_THREADS
	pthread_mutex_unlock(&(C->send.action_lock));
	pthread_cond_broadcast(&C->send.conn_cv);
	pbarrier_wait(&(C->send.conn_barrier));
#endif

    }
   
#ifdef CONN_THREADS
    pthread_mutex_unlock(&(C->send.conn_lock));
#endif
   
    FN_OUT_DEBUG("send_message");
    return 0;
}

/* size of appended blocks must exactly match the given size of the initial send_message */
int conn_send_message_block_append(void *sendbuf, size_t act_bufsize, int conn) {
    struct SConnection *C;
    struct SSocket *sle;
    
    
    FN_IN_DEBUG("send_message_block_append");
    
    if (conn >= num_conn) {
	RERROR2("%ssend_message: non-existent conn handle: %d\n", RDEBUG_dbgprefix, conn);
	return -1;
    }
    C = &connlist[conn];
    
    sle = &(C->sockets);
    
    /* data */
    if( C->write((void *)sendbuf, act_bufsize, sle->remotesocket, sle->sndMaxSduSize) != act_bufsize )
	return -1;
    
}

int conn_send_message(void *sendbuf, size_t bufsize, int conn) {
    return conn_send_message_block(sendbuf, bufsize, bufsize, conn);
}

/*******************************************************************************
conn_receive_message() receives data from network on connection conn and stores it 
in recvbuf. Bufsize indicates the size of recvbuf. It should be large enough 
to store received data. Otherwise, receive_message() fails.
One of the main tasks of receive_message() is to detect if message to receive 
is splitted and  wake up receive threads to receive parts of this splitted 
message, if the size of received message exceeds split_size. 
If the received message small enough, single-socket transmissions are handled 
without invoking the threads.
********************************************************************************/

int conn_receive_message(void *recvbuf, size_t bufsize, int conn)
{
   struct SSocket *sle;
   struct SConnection *C;
   size_t          pktsize;
   int             socket_bufsize;
   int             thrd;
   
   FN_IN_DEBUG("receive_message");
   
   if (conn >= num_conn) {
      RERROR2("%sreceive_message: non-existent conn handle: %d\n", RDEBUG_dbgprefix, conn);
      exit(-1);
      /* return -1; */
   }
   C = &connlist[conn];
   /*
   * read header of the message, if not already done. pktsize is the
   * size of the message that is due to be read.
   */
   pktsize = conn_check_message(conn, CHECK_BLOCK);
   if (pktsize < 0) {
      RERROR1("%sreceive_message: internal error\n",RDEBUG_dbgprefix);
      exit(-1);
   }
   if (pktsize > bufsize) {
      RDEBUG1("receive_message: buffer too small  on connection %d", conn);
      RDEBUG1(", need %d", pktsize);
      RDEBUG1(", have  %d", bufsize);
      exit(-1);
      /* return CONN_ERR_BUF; */
   }
#ifdef CONN_THREADS

   pthread_mutex_lock(&(C->recv.conn_lock));
#endif   
   /* get the rest of the message */
   
   sle = &(C->sockets);
   if (pktsize <= C->splitSize) {
      /* single-socket transmissions are handled without invoking the threads */
      C->read((void *)recvbuf, pktsize, sle->remotesocket, sle->rcvMaxSduSize);
   } else {
      /* how many sockets will receive parts of this mesage ? */
      C->recv.use_sockets = pktsize / C->splitSize + (pktsize % C->splitSize != 0);
      if (C->recv.use_sockets > C->nbr_sockets)
         C->recv.use_sockets = C->nbr_sockets;
      if (C->recv.use_sockets == 0)
         C->recv.use_sockets = 1;
      socket_bufsize = pktsize / C->recv.use_sockets;
      /* distribute message buffer on the 
      threads directly use the buffer given from above */
      sle = &(C->sockets);
      for (thrd = 0; thrd < C->recv.use_sockets; thrd++) {
         sle->recv_thread->net_buf = ((char *) recvbuf) + thrd * socket_bufsize;
         sle->recv_thread->buf_size = socket_bufsize;
         if (thrd == C->recv.use_sockets - 1)
            sle->recv_thread->buf_size += pktsize % C->recv.use_sockets;
         sle = sle->next;
      }
      /* wake up the threads */
#ifdef CONN_THREADS
      pthread_mutex_lock(&(C->recv.action_lock));
#endif
      C->recv.conn_action = READ;
#ifdef CONN_THREADS

      pthread_mutex_unlock(&(C->recv.action_lock));
      pthread_cond_broadcast(&C->recv.conn_cv);
      pbarrier_wait(&(C->recv.conn_barrier));
#endif
   }
   
   C->header_is_read = 0;
#ifdef CONN_THREADS

   pthread_mutex_unlock(&(C->recv.conn_lock));
#endif   
   FN_OUT_DEBUG("receive_message");
   
   return pktsize;
}


/* tcp_select_all slects on all sockets */
int select_all(int timeout)
{
   fd_set          rfds;
   struct timeval  tv, *ptv;
   int             retval;
   int             i;
   int             maxsocket;
   
   FD_ZERO(&rfds);
   
   maxsocket = -1;
   for (i = 0; i < num_conn; i++)
      if (connlist[i].state == CONNSTATE_UP) {
         FD_SET(connlist[i].sockets.remotesocket, &rfds);
         if (connlist[i].sockets.remotesocket > maxsocket)
            maxsocket = connlist[i].sockets.remotesocket;
      }
      if (timeout < 0)
         ptv = NULL;
      else {
         ptv = &tv;
         tv.tv_sec = 0;
         tv.tv_usec = timeout;
      }
      retval = select(maxsocket + 1, &rfds, NULL, NULL, ptv);
      if (retval < 0) {
         RERROR1("%sselect_all: error in select: ",RDEBUG_dbgprefix);
         perror("");
      }
      return retval;
}


int conn_select_connection(int conn, int timeout)
{
   fd_set          rfds;
   struct timeval  tv, *ptv;
   int             retval;
   
   FN_IN_DEBUG("select_connection");
   if (conn >= num_conn) {
      RERROR2("%sselect_connection: non-existent connection handle: %d\n", conn, RDEBUG_dbgprefix);
      FN_OUT_DEBUG("select_connection");
      return -1;
   }
   FD_ZERO(&rfds);
   FD_SET(connlist[conn].sockets.remotesocket, &rfds);
   
   if (timeout < 0)
      ptv = NULL;
   else {
      ptv = &tv;
      tv.tv_sec = 0;
      tv.tv_usec = timeout;
   }
   
   retval = select(connlist[conn].sockets.remotesocket + 1, &rfds, NULL, NULL, ptv);
   if (retval < 0) {
      RERROR2("%sselect_connection: error in select on connection %d: ", RDEBUG_dbgprefix, conn);
      perror("");
   }
   FN_OUT_DEBUG("select_connection");
   return retval;
}


void conn_remove_connections()
{
   int             i;
   struct SSocket *sle, *free_sle;
   
   for (i = 0; i < num_conn; i++)
      if (connlist[i].state != CONNSTATE_DOWN) {
         RERROR2("%sconn_remove_connections: connection %d is not down, closing ...\n", RDEBUG_dbgprefix, i);
         sle = &(connlist[i].sockets);
         while (sle != NULL) {
            close(sle->remotesocket);
            free_sle = sle;
            sle = sle->next;
            if (free_sle != &(connlist[i].sockets))
               free(free_sle);
         }
         connlist[i].sockets.next = NULL;
      }
      num_conn = 0;
      free(connlist);
}

int conn_check_message(int conn, int mode)
{
   struct SConnection *C;
   struct SSocket  *sle, *thrd_sle;
   int    thrd;
   
   FN_IN_DEBUG("check_message");
   
   if (conn >= num_conn) {
      RERROR2("%scheck_message: non-existent connection handle: %d\n", RDEBUG_dbgprefix, conn);
      return -1;
   }
   
   C = &connlist[conn];
#ifdef CONN_THREADS

   pthread_mutex_lock(&(C->recv.conn_lock));
#endif   
   if (!C->header_is_read) {
      if ((mode == CHECK_NONBLOCK) ? conn_select_connection(conn, 0) : 1) {
         /* read header of the message on the first socket */
         sle = &(C->sockets);
         if (C->read((void *) &(C->recv.header_buf), sizeof(struct SBuf), sle->remotesocket, sle->rcvMaxSduSize)
            != sizeof(struct SBuf)) {
            RERROR2("%scheck_message(): error reading message header on connection %d: ",
               RDEBUG_dbgprefix, conn);
            perror("");
            exit(-1);
         }
         /* convert network order to host order */
         /* could be implemented as a macor, depending on -hetero for configure */
         C->recv.header_buf.magic = ntohs(C->recv.header_buf.magic);
         C->recv.header_buf.size = ntohl(C->recv.header_buf.size);
         sle->recv_thread->buf_size = C->recv.header_buf.size;
         C->recv.header_buf.total_msgsize = ntohl(C->recv.header_buf.total_msgsize);
         
         if (C->recv.header_buf.magic != MAGIC_HEADER) {
            RERROR2("%scheck_message(): wrong magic number on connection %d\n", RDEBUG_dbgprefix, conn);
            exit(-1);
         }
         /* number of sockets to use is any between 1
         * and nbr_sockets depending on the actual message size */
         C->recv.use_sockets = C->recv.header_buf.total_msgsize / C->splitSize
            + (C->recv.header_buf.total_msgsize % C->splitSize != 0);
         if (C->recv.use_sockets > C->nbr_sockets)
            C->recv.use_sockets = C->nbr_sockets;
         if (C->recv.use_sockets == 0)
            C->recv.use_sockets = 1;
         C->recv.total_msgsize = C->recv.header_buf.total_msgsize;
         
         if (C->recv.use_sockets > 1) {
            thrd_sle = sle;
            for (thrd = 1; thrd < C->recv.use_sockets; thrd++) {
               thrd_sle = thrd_sle->next;			    
               thrd_sle->recv_thread->buf_size = sle->recv_thread->buf_size;
            }
            thrd_sle->recv_thread->buf_size += C->recv.header_buf.total_msgsize % 
               C->recv.use_sockets;
         }
         
      } else {
         FN_OUT_DEBUG("check_message");
         return -1;
      }
   }
   C->header_is_read = 1;
#ifdef CONN_THREADS

   pthread_mutex_unlock(&(C->recv.conn_lock));
#endif
   FN_OUT_DEBUG("check_message");
   
   return C->recv.total_msgsize;
}



void  conn_setSplitSize     (int conn, long  splitSize)  
{
  if ( conn >= num_conn  ) {
    RERROR1("conn_setSplitSize: invalid conn handle %d\n",conn); 
    exit(-1);
  }
  connlist[conn].splitSize=splitSize;
}


