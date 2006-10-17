/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "connection.h"
#include <sys/time.h>
#include "sequence.h"

#define MAX_PKT 1024*1024*32
#define PKT_START 1

#define SPLITSIZE 610
#define TCP_BASEPORT 2500
#define RETRIES 10
#define MAX_SOCKETS 8

#define FERTIG 1
#define WEITER 0

/*#define VERIFY 1*/
#define VERBOSE 1


/* calculate the elapsed time between two t1 and t2 in microseconds */
static double elapsed_time(struct timeval* t1, struct timeval* t2)
{
  return(((double)(t2->tv_sec - t1->tv_sec))*1000000.0
         +  (double)(t2->tv_usec - t1->tv_usec));
}

char sndbuf[MAX_PKT];  
char recbuf[MAX_PKT];

int main(int argc, char **argv) {
    int conn, i ,j;
    int split_size;
    struct sockaddr_in addr1[MAX_SOCKETS], addr2[MAX_SOCKETS];
    int server;
    int size;
    char serverAddr[255];
    struct hostent * he;
    double transmit_time;
    struct timeval t1;
    struct timeval t2;
    int retries,r;
    int c, nbr_sockets = 1, nbr_param;


    while((c = getopt(argc, argv, "s:h?")) != EOF) {
	switch(c) {
	case 's':
	    nbr_sockets = atoi(optarg);
	    break;
	case '?':
	case 'h':
	    printf ("Usage: %s [server address]\n", argv[0]);
	    printf ("  -s   number of sockets [default: 1]\n");
	    printf ("       Must be number of server addresses\n");
	    exit(1);
	    break;
	}
    }

    nbr_param = argc - optind;
    if ( nbr_param != nbr_sockets && nbr_param != 0 ) {
	printf ("\nwrong number of parameters\n%s -h for help\n", argv[0]);
	exit(1);
    }


    retries=RETRIES;
    split_size = 2500;
    
    for (i = 0; i < MAX_PKT; i++)
      sndbuf[i] = i;

    init_connections(split_size);


    if ( nbr_param >= 1) {
	/* I am a connecting client */
	struct in_addr intmp;
	server = 0;

	for ( i = 0; i < nbr_sockets; i++ ) {
	    strcpy(serverAddr, "");
	    if(!inet_pton(AF_INET,argv[ i + optind ],&intmp)) {
		he = gethostbyname(argv[ i + optind ]);
		if (he) {
		    memcpy(&intmp, he->h_addr, he->h_length);
		    sprintf(serverAddr, "%s", inet_ntoa(intmp));		
		    printf("address for %s is %s\n", argv[ i + optind], serverAddr);
		    fflush(stdout);
		}
	    }
	    else strcpy (serverAddr, argv[ i + optind ]);

	    memset(&addr1[i],0,sizeof(struct sockaddr_in));
	    memset(&addr2[i],0,sizeof(struct sockaddr_in));
	    addr1[i].sin_addr.s_addr = INADDR_ANY;
	    addr2[i].sin_addr.s_addr = inet_addr(serverAddr);
	    addr1[i].sin_family = AF_INET;
	    addr2[i].sin_family = AF_INET;
	    addr1[i].sin_port = htons(2500 + i);
	    addr2[i].sin_port = htons(2500 + i);
	}


    } else {
	/* I am the one and only server */
	server = 1;
	for ( i = 0; i < nbr_sockets; i++ ) {
	    strcpy(serverAddr, "");
	    memset(&addr1[i],0,sizeof(struct sockaddr_in));
	    memset(&addr2[i],0,sizeof(struct sockaddr_in));
	    addr1[i].sin_addr.s_addr = inet_addr(serverAddr);
	    addr2[i].sin_addr.s_addr = INADDR_ANY;
	    addr1[i].sin_family = AF_INET;
	    addr2[i].sin_family = AF_INET;
	    addr1[i].sin_port = htons(2500 + i);
	    addr2[i].sin_port = htons(2500 + i);
	}
    }

    /* get connection: for client and server */
    conn = add_connection((struct sockaddr*)&addr1,(struct sockaddr*)&addr2, AF_INET, NULL, server?CONN_SERVER:CONN_CLIENT);

    for ( i=1; i < nbr_sockets ; i++ ) {
	add_socket( conn, (struct sockaddr*)&addr1[i], (struct sockaddr*)&addr2[i], NULL);
    }
    
    if (establish_connection(conn) < 0) {
      printf("kann Verbindung nicht aufbauen!\n");
      exit(-1);
    }

    /* send and receive */
    if (!server) printf ("[packet size] [lateny in usec] [bandwidth kb/s]\n");
    for (i = PKT_START; i <= MAX_PKT; i *= 2) {
      double timesum=0;	
      for (j = 0; j < retries; j++){
	  if (server) {
	      size = receive_message((char *)recbuf, MAX_PKT * sizeof(char), conn);
	      /* pong back immediatly */
	      send_message((char*)recbuf, size, conn);
	      if (size != i*sizeof(int)) {
		  fprintf(stderr, "expected %d Bytes, got %d Bytes\n", i*sizeof(int), size); fflush(stderr);
	      } 
	  }
	  else {
	      gettimeofday(&t1, NULL);  

	      send_message((char *)sndbuf, i*sizeof(int), conn);
	      size = receive_message((char *)sndbuf, MAX_PKT*sizeof(char), conn);
	      gettimeofday(&t2, NULL);  
#ifdef VERIFY
	      for ( j = 0; j < size / sizeof(char); j++) 
		  if (sndbuf[j] != j) {
		      printf ("error in data:  pos = %d!\n",j);
		      break;
		  }
#endif
	      /* write results into file */
	      transmit_time =  elapsed_time(&t1, &t2);
	      timesum+=transmit_time;	      
	  }
      }
      if (!server) 
	  printf("%11d %17.2f %16.3f\n", size, timesum/retries, 2*size*retries/timesum*1000000/1024);
      timesum=0;


    }

    /* bad hack, termination has to be cleaned up */
    sleep (1);
    close_connections(conn);	
    return 0;
}
