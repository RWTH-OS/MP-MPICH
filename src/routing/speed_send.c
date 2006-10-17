#include <stdio.h>
#include <unistd.h>

#include "connection.h"
#include "client_server.h"

/* calculate the elapsed time between two t1 and t2 in seconds */
static double elapsed_time(struct timeval* t1, struct timeval* t2)
{
  return((double)(t2->tv_sec - t1->tv_sec)
         + 0.000001 * (double)(t2->tv_usec - t1->tv_usec));
}

int sndbuf[PKTSIZE];  
int recbuf[PKTSIZE];

int main(int argc, char **argv) {
    int conn, i ,j;
    int nbr_ints;
    int split_size;
    struct sockaddr_in addr1, addr2;
    int server;
    int size;


    /* init */
    /*    if (argc < 2) {
      printf ("Usage: speed_send [-s] splitsize [nbr_sockets]\n");
      exit(1);
      }*/
    server=0;
    if (argc > 1) {
	if (strcmp(argv[1],"-s")==0) {
	    printf("waiting for client connection ...\n");
	    server=1;
	}
    }

    nbr_ints = PKTSIZE / sizeof (int);
    
    split_size = 2500; /*atoi(argv[1]);*/

    for (i = 0; i < nbr_ints; i++)
      sndbuf[i] = i;

    init_connections(split_size);

    memset(&addr1,0,sizeof(struct sockaddr_in));
    memset(&addr2,0,sizeof(struct sockaddr_in));

    if(server) {
	addr1.sin_addr.s_addr = inet_addr(IP_SERVER1);
	addr2.sin_addr.s_addr = inet_addr(IP_CLIENT1);
    } else {
	addr1.sin_addr.s_addr = inet_addr(IP_CLIENT1);
	addr2.sin_addr.s_addr = inet_addr(IP_SERVER1);
    }

    addr1.sin_family = AF_INET;
    addr2.sin_family = AF_INET;
    addr1.sin_port = htons(2500);
    addr2.sin_port = htons(2500);

    conn = add_connection((struct sockaddr*)&addr1,(struct sockaddr*)&addr2, AF_INET, NULL, server?CONN_SERVER:CONN_CLIENT);
#ifdef bla
    if (argc == 3) {
      if (atoi(argv[2]) > 1)
	add_socket(conn, IP_CLIENT2, TCP_BASEPORT + 1, IP_SERVER2, TCP_BASEPORT + 1);
      if (atoi(argv[2]) > 2)
	add_socket(conn, IP_CLIENT3, TCP_BASEPORT + 2, IP_SERVER3, TCP_BASEPORT + 2);
      if (atoi(argv[2]) > 3)
	add_socket(conn, IP_CLIENT4, TCP_BASEPORT + 3, IP_SERVER4, TCP_BASEPORT + 3);
      if (atoi(argv[2]) == 5)
	add_socket(conn, IP_CLIENT5, TCP_BASEPORT + 4, IP_SERVER5, TCP_BASEPORT + 4);
    }
#endif
    if (establish_connection(conn) < 0) {
      printf("kann Verbindung nicht aufbauen!\n");
      exit(-1);
    }

    for (i = PKT_START; i <= nbr_ints; i *= 2) {
      for (j = 0; j < NBR_PKTS; j++)
	  if (server) {
	      size = receive_message((char *)recbuf, PKTSIZE*sizeof(int), conn);
	      if (size != i*sizeof(int)) {
		  fprintf(stderr, "expected %d Bytes, got %d Bytes\n", i, size); fflush(stderr);
	      } 
	  }
	  else
	      send_message((char *)sndbuf, i*sizeof(int), conn);
#ifdef VERBOSE
      fprintf(stderr, "[ %d bytes sent]  ", i*sizeof(int));
      fflush(stderr);
#endif		
    }

    /* bad hack, termination has to be cleaned up */
    sleep (10);
    close_connections(conn);	
    return 0;
}
