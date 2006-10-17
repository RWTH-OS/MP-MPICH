/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#ifndef __TCP_CONNECTION_H
#define __TCP_CONNECTION_H

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/tcp.h>
#include <netinet/in.h>
#endif

#include "rdebug.h"

#ifndef CONN_SERVER
#define CONN_SERVER 1
#endif

#ifndef CONN_CLIENT  
#define CONN_CLIENT 0
#endif

/* size of the tcp sockets receive and send buffer */
#define SOCKET_BUF_SIZE 1024*1024
/* we want to try it 5 minutes - 500 ms * 600 tries */
/* tries to connect to server */
#define CONN_CONNECT_TRIES    600
/* time between tries */
#define CONN_CONNECT_TIMEOUT  500000

/* error codes */
#define ERR_TCP_CONN_BIND       -1
#define ERR_TCP_CONN_LISTEN     -2
#define ERR_TCP_CONN_ACCEPT     -3
#define ERR_TCP_CONN_CONNECT    -4
#define ERR_TCP_CONN_SOCKET     -5
#define ERR_TCP_CONN_SETSOCKOPT -6

int tcp_read(void *buffer, unsigned size, int fd, unsigned long maxSDUsize);
int tcp_write(void *buffer, unsigned size, int fd, unsigned long maxSDUsize);
int tcp_establish_sock_conn(int *localsocket_fd, struct sockaddr *local_addr, 
                            int *remotesocket_fd, struct sockaddr *remote_addr, 
                            char *qos,
                            unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize,
                            int max_nconn, int serv, int timeout);
int tcp_add_socket_pair (int *localsocket_fd, int *remotesocket_fd, 
                         unsigned short sa_family, int serv);


static int tcp_setsockopt(int sfd);

#endif
