/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#include <stdio.h>
#include <unistd.h>

#include "tcp_connection.h"
#include "client_server.h"


/* calculate the elapsed time between two t1 and t2 in seconds */
static double elapsed_time(struct timeval* t1, struct timeval* t2)
{
  return((double)(t2->tv_sec - t1->tv_sec)
         + 0.000001 * (double)(t2->tv_usec - t1->tv_usec));
}

int recbuf[PKTSIZE];

int main(int argc, char **argv){
  double transmit_time;
  int size, conn;
  int nbr_ints, nbr_channels, nbr_pkts;
  int j ,i;
  int split_size;
  FILE *results;

  struct timeval t1;
  struct timeval t2;
  
  /* init */
  if (argc < 2) {
    printf ("Usage: speed_recv splitsize [nbr_sockets]\n");
    exit(1);
  }
  nbr_ints = PKTSIZE / sizeof (int);
  split_size = atoi(argv[1]);
  nbr_pkts   = NBR_PKTS;
  nbr_channels = 1;
  
  results = fopen ("speed_results.txt", "w");
  init_connections(split_size);
  
  conn = add_connection(IP_SERVER1, TCP_BASEPORT, IP_CLIENT1, TCP_BASEPORT, CONN_SERVER);
  if (argc == 3) {
    if (atoi(argv[2]) > 1)
      add_socket(conn, IP_SERVER2, TCP_BASEPORT + 1, IP_CLIENT2, TCP_BASEPORT + 1);
    if (atoi(argv[2]) > 2)
      add_socket(conn, IP_SERVER3, TCP_BASEPORT + 2, IP_CLIENT3, TCP_BASEPORT + 2);
    if (atoi(argv[2]) > 3)
      add_socket(conn, IP_SERVER4, TCP_BASEPORT + 3, IP_CLIENT4, TCP_BASEPORT + 3);
    if (atoi(argv[2]) == 5)
      add_socket(conn, IP_SERVER5, TCP_BASEPORT + 4, IP_CLIENT5, TCP_BASEPORT + 4);
    nbr_channels = atoi(argv[2]);
  }
  if (establish_connection(conn) < 0) {
    printf("kann Verbindung nicht aufbauen!\n");
    exit(-1);
  }
  fprintf (results, "# %d channels, splitsize = %d, nbr_pkts = %d\n", 
	   nbr_channels, split_size, nbr_pkts);
  
  for (i = PKT_START; i <= nbr_ints; i *= 2) {
#ifdef FOO
      while (!tcp_select(conn,-1)) {  
#ifdef VERBOSE
      printf("."); 
      fflush(stdout);
#endif		
    }
#endif
    
    gettimeofday(&t1, NULL);  
    for (j = 0; j < NBR_PKTS; j++) {
      size = receive_message((char *)recbuf, PKTSIZE*sizeof(int), conn);
      if (size != i*sizeof(int)) {
	fprintf(stderr, "expected %d Bytes, got %d Bytes\n", i, size); fflush(stderr);
      } 
    }
    gettimeofday(&t2, NULL);  

#ifdef VERIFY
    for ( j = 0; j < size / sizeof(int); j++) 
      if (recbuf[j] != j) {
	printf ("error in data:  pos = %d!\n",j);
	break;
      }
#endif

    /* write results into file */
    transmit_time =  elapsed_time(&t1, &t2);
    fprintf (results, "%7d %5.2f\n", size, size*NBR_PKTS/(1024*transmit_time));

#ifdef VERBOSE
    fprintf(stderr, "[ %d bytes o.k. ] ", size); fflush(stderr);
#endif		
    
  }
  
  printf ("\n\nZeit = %.6f s\n", elapsed_time(&t1, &t2));
  
  close_connections(conn);	
  fclose (results);
  return 1;
}
