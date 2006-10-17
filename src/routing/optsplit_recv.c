/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */

#include <stdio.h>
#include <unistd.h>

#include "tcp_connection.h"
#include "client_server.h"

/* optsplit_send & optsplit_recv are used to determine the optimum 
   splitsize for a multi-socket connection.

   The current version does this only for the transition from 
   one socket to two sockets. However, I assume that the thereby
   found value will give good results for higher-level transitions,
   too.

   Joachim
*/


/* calculate the elapsed time between two t1 and t2 in seconds */
static double elapsed_time(struct timeval* t1, struct timeval* t2)
{
  return((double)(t2->tv_sec - t1->tv_sec)
         + 0.000001 * (double)(t2->tv_usec - t1->tv_usec));
}

int recbuf[PKTSIZE];

int main(int argc, char **argv){
  double transmit_time, single_transmit_time, multi_transmit_time;
  int size, conn;
  int nbr_ints, nbr_channels, nbr_pkts, msg_inc;
  int found_ss;
  int j ,i;
  FILE *results;

  struct timeval t1;
  struct timeval t2;
  
  /* initialization */
  nbr_ints   = PKTSIZE / sizeof (int);
  split_size = PKTSIZE + 1;
  nbr_pkts   = NBR_PKTS;
  msg_inc = PKT_INC;
  nbr_channels = 2;
  found_ss = 0;
     
  results = fopen ("speed_results.txt", "w");
  init_connections();
  
  conn = add_connection(IP_SERVER1, TCP_BASEPORT + 0, IP_CLIENT1, TCP_BASEPORT + 0, CONN_SERVER);
  add_socket(conn, IP_SERVER2, TCP_BASEPORT + 1, IP_CLIENT2, TCP_BASEPORT + 1);
  if (argc == 2) {
      if (atoi (argv[1]) > 2)
	  add_socket(conn, IP_SERVER3, TCP_BASEPORT + 2, IP_CLIENT3, TCP_BASEPORT + 2);
      if (atoi (argv[1]) > 3)
	  add_socket(conn, IP_SERVER4, TCP_BASEPORT + 3, IP_CLIENT4, TCP_BASEPORT + 3);
      if (atoi (argv[1]) == 5)
	  add_socket(conn, IP_SERVER5, TCP_BASEPORT + 4, IP_CLIENT5, TCP_BASEPORT + 4);
      nbr_channels = atoi(argv[1]);
  }
  if (tcp_establish_connection(conn) < 0) {
    printf("kann Verbindung nicht aufbauen!\n");
    exit(-1);
  }
  fprintf (results, "# %d channels, splitsize to be evaluated, nbr_pkts = %d\n", 
	   nbr_channels, split_size, nbr_pkts);
  
  for (i = PKT_START; i <= nbr_ints; i += msg_inc/sizeof(int)) {
    while (!tcp_select(conn,-1)) {  
#ifdef VERBOSE
      printf("."); 
      fflush(stdout);
#endif		
    }
    
    if (!found_ss) {
	split_size = i*sizeof (int);
	gettimeofday(&t1, NULL);
	for (j = 0; j < NBR_PKTS; j++) {
	    size = receive_message((char *)recbuf, PKTSIZE*sizeof(int), conn);
	    if (size != i*sizeof(int)) {
		fprintf(stderr, "expected %d Bytes, got %d Bytes\n", i, size); fflush(stderr);
	    } 
	}
	gettimeofday(&t2, NULL);  
	single_transmit_time = elapsed_time(&t1, &t2);
    
#ifdef VERIFY
    for ( j = 0; j < size / sizeof(int); j++) 
      if (recbuf[j] != j) {
	printf ("error in data (single socket):  pos = %d!\n",j);
	    break;
      }
#endif
    }

    if (!found_ss)
	split_size = i*sizeof (int)/2;
    gettimeofday(&t1, NULL);
    for (j = 0; j < NBR_PKTS; j++) {
      size = receive_message((char *)recbuf, PKTSIZE*sizeof(int), conn);
      if (size != i*sizeof(int)) {
	fprintf(stderr, "expected %d Bytes, got %d Bytes\n", i, size); fflush(stderr);
      } 
    }
    gettimeofday(&t2, NULL);  
    multi_transmit_time = elapsed_time(&t1, &t2);

#ifdef VERIFY
    for ( j = 0; j < size / sizeof(int); j++) 
      if (recbuf[j] != j) {
	printf ("error in data (multi socket):  pos = %d!\n",j);
	    break;
      }
#endif
    /* result output */
    if (!found_ss) {
	if ((single_transmit_time - multi_transmit_time) > 0.05 * single_transmit_time) {
	    printf ("\nSplitsize %d seems to be a good value for %d sockets\n => will continue with it \n", 
		    split_size, nbr_channels);
	    fprintf (results, "# splitsize set to %d bytes \n", split_size);
	    /* tell sender to continue with this splitsize */
	    found_ss = 1;
	    msg_inc *= 8;
	} 
	tcp_send_message((char *)&found_ss, sizeof(found_ss), conn);
    }
    transmit_time = found_ss ? multi_transmit_time : single_transmit_time;
    fprintf (results, "%7d %5.2f\n", size, size*NBR_PKTS/(1024*transmit_time));

#ifdef VERBOSE
    fprintf(stderr, "[ %d bytes o.k. ] ", size); fflush(stderr);
#endif		
    
  }
  
  tcp_close_connections(conn);	
  fclose (results);
  return 1;
}
