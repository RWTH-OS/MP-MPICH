/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#include <stdio.h>
#include "connection.h"
#include "client_server.h"


/* optsplit_send & optsplit_recv are used to determine the optimum 
   splitsize for a multi-socket connection.

   The current version does this only for the transition from 
   one socket to two sockets. However, I assume that the thereby
   found value will give good results for higher-level transitions,
   too.

   Joachim
*/

int sndbuf[PKTSIZE];  


int main(int argc, char **argv) {
    int conn, i ,j;
    int nbr_ints, msg_inc;
    int found_ss;

    struct timeval t1;
    struct timeval t2;

    struct sockaddr_in addr1, addr2,addr3,addr4;
    
    memset(&addr1,0,sizeof(struct sockaddr_in));
    memset(&addr2,0,sizeof(struct sockaddr_in));
    addr1.sin_addr.s_addr = inet_addr(IP_CLIENT1);
    addr2.sin_addr.s_addr = inet_addr(IP_CLIENT1);
    addr1.sin_family = AF_INET;
    addr2.sin_family = AF_INET;
    addr1.sin_port = htons(TCP_BASEPORT + 0);
    addr2.sin_port = htons(TCP_BASEPORT + 0);

    /* XXX these to parameters */
    nbr_ints = PKTSIZE / sizeof (int);
    msg_inc = PKT_INC;
    found_ss = 0;
    
    for (i = 0; i < nbr_ints; i++)
      sndbuf[i] = i;
    conn_init_connections(2500);

    conn = conn_add_connection(&addr1,&addr2,AF_INET, 0, CONN_CLIENT, 60);
    conn_add_socket(conn, IP_CLIENT2, TCP_BASEPORT + 1, IP_SERVER2, TCP_BASEPORT + 1);
    if (argc == 2) {
      if (atoi (argv[1]) > 2)
	conn_add_socket(conn, IP_CLIENT3, TCP_BASEPORT + 2, IP_SERVER3, TCP_BASEPORT + 2);
      if (atoi (argv[1]) > 3)
	conn_add_socket(conn, IP_CLIENT4, TCP_BASEPORT + 3, IP_SERVER4, TCP_BASEPORT + 3);
      if (atoi (argv[1]) == 5)
	conn_add_socket(conn, IP_CLIENT5, TCP_BASEPORT + 4, IP_SERVER5, TCP_BASEPORT + 4);
    }
    if (conn_establish_connection(conn) < 0) {
      printf("kann Verbindung nicht aufbauen!\n");
      exit(-1);
    }

    for (i = PKT_START; i <= nbr_ints; i += msg_inc/sizeof(int)) {
	if (!found_ss) {
	  conn_setSplitSize(  i * sizeof (int));
	    for (j = 0; j < NBR_PKTS; j++)
		conn_send_message((char *)sndbuf, i*sizeof(int), conn);
#ifdef VERBOSE
	    fprintf(stderr, "[ %d bytes sent]  ", i*sizeof(int));
	    fflush(stderr);
#endif		

	    conn_setSplitSize(   i*sizeof(int)/2);
	}

	for (j = 0; j < NBR_PKTS; j++)
	    conn_send_message((char *)sndbuf, i*sizeof(int), conn);
#ifdef VERBOSE
	fprintf(stderr, "[ %d bytes sent]  ", i*sizeof(int));
	fflush(stderr);
#endif		
	if (!found_ss){
	  receive_message((char *)&found_ss, sizeof(found_ss), conn);
	  if (found_ss)
	    msg_inc *= 8;
	}
    }
    
    tcp_close_connections(conn);	
    return 0;
}
