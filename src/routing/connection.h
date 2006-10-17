/* $Id$
  connection.h
  these functions provide a simple interface to bsd sockets
 */
  
#ifndef __CONNECTION_H
#define __CONNECTION_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#define bzero(a,b) memset(a,0,b)
#else
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <netdb.h>
#include <strings.h>
#endif


#include <unistd.h>
#include <pthread.h>


#include "rdebug.h"
#include "pbarrier.h"

#include "conn_common.h"
#include "tcp_connection.h"

#ifdef META_ATM

#include "atm_connection.h"
#endif 

/* at startup, send and receive buffer have this size */
#define INITIAL_BUFFER_SIZE 100

#ifdef bla
#ifndef CONN_SERVER
#define CONN_SERVER 1
#endif

#ifndef CONN_CLIENT  
#define CONN_CLIENT 0
#endif
#endif
/* maximum number of connections in connlist */
/*#define MAX_CONN_COUNT 20 */
#define MAX_CONN_COUNT 5

/* maximal number of connections used for the listen call */
#define MAX_LISTEN_CONN 1


/* this is the maximun size of the tcp-packets */
#define IP_STRING_LENGTH 256

/* error codes for conn_conn_establish_connection() */
#define CONN_ERR_NO_CONN     -1

/*
  #define CONN_ERR_NO_SOCKET   -2
  #define CONN_ERR_PACKET_SIZE -3
  #define CONN_ERR_ACCEPT      -4
  #define CONN_ERR_LISTEN      -5
  #define CONN_ERR_SOCKET      -6
  #define CONN_ERR_IP          -7
  #define CONN_ERR_WRITE       -8
  #define CONN_ERR_CONNECT     -9
*/

#define CONN_ERR_READ       -11
#define CONN_ERR_BUF        -12

#define CONNSTATE_UP     1
#define CONNSTATE_DOWN   2
#define CONNSTATE_BOUND  3
#define CONNSTATE_LISTEN 4

/* modes for the conn_conn_check_message function */
#define CHECK_NONBLOCK 0
#define CHECK_BLOCK 1

#define MAGIC_HEADER 12345

typedef enum _conn_thread_action { READ, WRITE, HEADER, WAIT, RESYNC, ADDSOCK, QUIT } conn_thread_action;


int  conn_init_connections     (int splitSize);
void  conn_setSplitSize     (int conn, long splitSize);
int  conn_add_connection       (struct sockaddr *local_addr, 
                           struct sockaddr *remote_addr,
			   unsigned short  sa_family,
                           char *qos, 
				int serv,
				int timeout);

int  conn_add_socket           (int conn, 
                           struct sockaddr *local_addr, 
                           struct sockaddr *remote_addr, 
                           char *qos);

void conn_close_connections    ();
int  conn_establish_connection (int conn);
int  conn_getnumconn           ();
int  conn_get_conn_type        (int conn);
void conn_remove_connections   ();
int conn_select_connection(int conn, int timeout);
int conn_check_message(int conn, int mode);
int conn_send_message(void *sendbuf, size_t bufsize, int conn);
int conn_send_message_block(void *sendbuf, size_t total_bufsize, size_t act_bufsize, int conn);
int conn_send_message_block_append(void *sendbuf, size_t act_bufsize, int conn);
int conn_receive_message(void *recvbuf, size_t bufsize, int conn);

#endif
