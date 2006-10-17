/*
 * $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>

#ifndef WIN32
#include <unistd.h>
int close(int fd);
#endif

#include "tcp_connection.h"
#include <errno.h>
#ifdef WIN32

#define  usleep(x) Sleep( (unsigned int) (x /1000))
#define ECONNREFUSED            WSAECONNREFUSED

#endif

 /* return value: 
   =  0: OK
   = -1: ERROR
 */
int tcp_listen(int socket_fd, int nbr)
{
   return listen(socket_fd, nbr);
}

/* return value: 
   =  0: OK
   = -1: ERROR
 */
int tcp_bind(int *localsocket_fd, struct sockaddr_in *local_addr, 
             int *remotesocket_fd, struct sockaddr_in *remote_addr, int serv)
{
   struct sockaddr_in addr;
   int fd;

   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   if (serv == CONN_CLIENT) {
      addr.sin_port = 0;
      addr.sin_addr.s_addr = local_addr->sin_addr.s_addr;
      fd = *remotesocket_fd;
   }
   else {
      addr.sin_port = local_addr->sin_port;
      /* convert from host byte order to network byte order */
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      fd = *localsocket_fd;
   }
   if (bind(fd, (struct sockaddr *) & (addr), sizeof(addr)) == -1) {
      PRERROR("tcp_bind: error binding socket\n"); perror("");
      return -1;
   }
   return 0;
}

/* return value: 
   =  0: OK
   = -1: ERROR
 */

int tcp_select(int sockfd, int wait) {
   fd_set          rfds;
   struct timeval  tv, *ptv;
   int             retval;

   FD_SET(sockfd, &rfds);
   
   if (wait < 0)
      ptv = NULL;
   else {
      ptv = &tv;
      tv.tv_sec = wait;
      tv.tv_usec = 0;
   }
   PRVERBOSE("checking socket...\n");
   
   retval = select(sockfd+1, &rfds, NULL, NULL, ptv);

   return retval;
}

int tcp_accept(int localsocket_fd, int *remotesocket_fd, int timeout)
{
   int             new_sd;
   PRVERBOSE1("accepting...for %ds\n", timeout);
   if (timeout==0) 
     timeout=-1; /* select will wait forever */

   if (tcp_select(localsocket_fd, timeout) != 0) {
     new_sd = accept(localsocket_fd, NULL, NULL);
     if (new_sd == -1) {
       PRERROR("tcp_accept: accept error\n");
       perror("");
       return -1;
     }
     *remotesocket_fd = new_sd;
     close(localsocket_fd);
     /*   connlist[conn].state = CONNSTATE_UP; */
     PRVERBOSE("connected to client!\n");
   }
   else {
     PRERROR("timeout for server exceeded!\n");
     return -1;
   }
   return 0;
}

/* return value: 
   =  0: OK
   = -1: ERROR
 */
int tcp_connect(int *remotesocket_fd, struct sockaddr_in *addr, int *connect_error)
{
   int             result;
   char            text[255];
   int             try;
   
   sprintf(text, "connecting to remotesocket %d on port %d ip %s\n",
           remotesocket_fd, ntohs(addr->sin_port), inet_ntoa (addr->sin_addr));
   PRVERBOSE2("connecting to  %s:%d\n", inet_ntoa (addr->sin_addr), ntohs(addr->sin_port));
   /* connect to server */
   result = connect(*remotesocket_fd, (const struct sockaddr *) addr, sizeof(struct sockaddr_in));
   if (result == 0) {
     PRVERBOSE2("successfully connected to server %s:%d\n", inet_ntoa (addr->sin_addr), ntohs(addr->sin_port));
   }
   else {
     *connect_error = errno;
     if ( errno == ECONNREFUSED ) {
	 /*RDEBUG(".");*/     
     }
     else {
       perror("");
     }
   }
   return result;
}  

/* return value:
   number of bytes received on socket fd
 */
int tcp_read(void *buffer, unsigned size, int fd, unsigned long maxSDUsize)
{
   int             r;
   unsigned        s = size;
   char           *pos = (char *) buffer;       
   
   FN_IN_DEBUG("tcp_read");
   do {
      r = recv(fd, pos, s, 0);
      if (r == 0)
         PRDEBUG("tcp_read: 0 Bytes received\n");
      if (r > 0) {
         s -= r;
         pos += r;
      } else {
         FN_OUT_DEBUG("tcp_read");
         return (size - s);
      }
   } while (s > 0);
   
   FN_OUT_DEBUG("tcp_read");
   return (size - s);
}

/* return value:
   number of bytes sent on socket fd
 */
int tcp_write(void *buffer, unsigned size, int fd, unsigned long maxSDUsize)
{
   int             w;
   unsigned        s = size;
   char           *pos = (char *) buffer;
   
   FN_IN_DEBUG("tcp_write");
   
   do {
      w = send(fd, pos, s, 0);
      if (w == 0)
         PRDEBUG("tcp_write: 0 Bytes received\n");
      if (w > 0) {
         s -= w;
         pos += w;
      } else {
         FN_OUT_DEBUG("tcp_write");
         return (size - s);
      }
   } while (s > 0);
   
   FN_OUT_DEBUG("tcp_write");
   return (size - s);
}


/* return value
   =  0: OK
   <  0: ERROR
 */
int tcp_add_socket_pair (int *localsocket_fd, int *remotesocket_fd, 
						 unsigned short sa_family, int serv)
{
   if (serv == CONN_SERVER) {
      /* create socket */
      if ((*localsocket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         PRERROR("tcp_add_socket_pair: cannot create local socket\n");
         return ERR_TCP_CONN_SOCKET;
      }
      if (tcp_setsockopt(*localsocket_fd) == -1) {
         PRERROR("tcp_add_socket_pair: cannot set socket options\n");
         return ERR_TCP_CONN_SETSOCKOPT;
      }
      *remotesocket_fd = -1;
   } else {
      /* create socket */
      if ((*remotesocket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         PRERROR("tcp_add_socket_pair: cannot create remote socket\n");
         return ERR_TCP_CONN_SOCKET;
      }
      if (tcp_setsockopt(*remotesocket_fd) == -1) {
         PRERROR("tcp_add_socket_pair: cannot set socket options\n");
         return ERR_TCP_CONN_SETSOCKOPT;
      }
      *localsocket_fd = -1;
   }
   return 0;
}

/* return value: 
   =  0: OK
   = -1: ERROR
 */
static int tcp_setsockopt(int sfd)
{
   int             value;

   value = 1;
   if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value)) != 0)
      return -1;
   value= SOCKET_BUF_SIZE;
   if (setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char *) &value, sizeof(value)) != 0)
      return -1;
   if (setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (char *) &value, sizeof(value)) != 0)
      return -1;
   value = 1;
   if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value)) != 0)
      return -1;
   return 0;
}

/* return value:
   = 0: OK
   < 0: ERROR
 */
int tcp_establish_sock_conn 
(
 int *localsocket_fd,          /* descriptor of local socket                    */
 struct sockaddr *local_addr,  /* local internet address (struct *sockaddr_in)  */
 int *remotesocket_fd,         /* descriptor of remote socket                   */
 struct sockaddr *remote_addr, /* remote internet address (struct *sockaddr_in) */
 char *qos,                    /* parameter ignored                             */
 unsigned long *rcvMaxSduSize, /* parameter ignored                             */
 unsigned long *sndMaxSduSize, /* parameter ignored                             */
 int max_nconn,                /* the maximum length of the queue of pending    */
                               /* connections, passed to call of listen()       */
 int serv,                     /* active (server) or passive (client) open connection */
 int timeout                   /* timeout for connect 0=wait forever*/
 )
 
{
   if (serv == CONN_SERVER) {
      /* server accepts */
      if (tcp_bind(localsocket_fd,(struct sockaddr_in *)local_addr,
		  remotesocket_fd,(struct sockaddr_in *)remote_addr,serv) == -1) {
	PRERROR("tcp_establish_sock_conn: error in tcp_bind on port");
	perror("");
         return ERR_TCP_CONN_BIND;
      }
      if (tcp_listen(*localsocket_fd, max_nconn) == -1) {
         PRERROR("listen_connection: error in listen\n"); perror("");
         return ERR_TCP_CONN_LISTEN;
      }
      PRVERBOSE2("accepting from local adress %s:%d\n",  inet_ntoa (((struct sockaddr_in *)local_addr)->sin_addr),
	       ntohs( ((struct sockaddr_in *)(local_addr))->sin_port) );
     
      if (tcp_accept(*localsocket_fd, remotesocket_fd, timeout) == -1) {
         PRERROR("tcp_establish_sock_conn(): error in tcp_accept\n");
	 close(*localsocket_fd);
         return ERR_TCP_CONN_ACCEPT;
      }
   } else {
     int result=-1, connect_error;
     int tries=timeout*1000*1000/CONN_CONNECT_TIMEOUT;
     
     if (timeout==0) tries=INT_MAX;
     PRVERBOSE1("trying %d times\n",tries);
     while ( result == -1 && tries > 0 ) {
       /* client connects */
       if (tcp_bind(localsocket_fd, (struct sockaddr_in *)local_addr, 
		    remotesocket_fd, (struct sockaddr_in *)remote_addr, serv) == -1) {
         PRERROR("tcp_establish_sock_conn(): error in tcp_bind\n");
         return ERR_TCP_CONN_BIND;
       }
       result=tcp_connect(remotesocket_fd, (struct sockaddr_in *)remote_addr, &connect_error );
       
       if (result == -1) {
	 if (connect_error != ECONNREFUSED) {
	   PRERROR("tcp_establish_sock_conn(): error in tcp_connect\n");
	   return ERR_TCP_CONN_CONNECT;
	 } else {
	   tcp_add_socket_pair (localsocket_fd, remotesocket_fd, AF_INET, serv);
	   usleep(CONN_CONNECT_TIMEOUT);
	 }
       }
       tries--;
     } /* while */
     if ( result == -1) { /* no luck connecting .... */
	 PRERROR2("could not connect to  %s:%d\n",  inet_ntoa (((struct sockaddr_in *)remote_addr)->sin_addr),
	       ntohs( ((struct sockaddr_in *)(remote_addr))->sin_port) );
	 return ERR_TCP_CONN_CONNECT;
     }
   }
   return 0;
}
