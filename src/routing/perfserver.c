/*
 * $Id: perfserver.c,v 1.8 2004/03/26 09:41:42 boris Exp $
 *
 * by Marko Koscak
 * 28.5.2001
 *
 * This code implements the Performance-Monitor Server. The Server is
 * directly involved to the mpi_router process.
 *
 * It exports router performance data via a TCP/IP socket to one or more remote
 * client programs.
 *
 */
 
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef WIN32
#include "pthread.h"
#include <windows.h>
#include <time.h>
#else
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

#endif
/* win32 not supported yet!*/
#ifndef WIN32
#include "perfserver.h"


#define		PERFSERV_TCP_PORT			5000

pthread_mutex_t	perfcalc_lock;
pthread_cond_t	perfcalc_cv;

/* global variables for the performance-Monitor Server */
static pthread_mutex_t perfsend_lock;
static pthread_cond_t	perfsend_cv;

/* mutex for the router_data and router_load variables */
/*pthread_mutex_t router_data_lock;
pthread_mutex_t router_load_lock;
*/

unsigned long int	router_load; /* the variable used by the sendthreads */

/* pointers to the two main perf threads */
pthread_t perfcalc_thread, perfconn_thread;

/* used to tell all threads to quit */
int	perf_app_status;

int		router_data;

/* states of the perfserver */
#define		PERF_APP_RUNNING	0
#define		PERF_APP_FINALIZE	1


 
/* a thread, which calculates the router throughput and nothing other */
void *perfcalculate_thread( void *null ) {
	struct				timeval now;
	struct				timeval last;

	long int			data;			/* local countvariable */
	unsigned long int	router_multi;
	unsigned long int	router_help;
	unsigned long int	time;
	unsigned long int	utime;
	
	gettimeofday(&last, NULL);	/* init last timestamp */

	PDEBUG( "calc_thread: start running\n" );				/* DEBUG */


	/* run until application exits */
	while( perf_app_status == PERF_APP_RUNNING ) {
		/* wait for signal */
		pthread_mutex_lock(&perfcalc_lock);
		pthread_cond_wait( &perfcalc_cv, &perfcalc_lock );
		pthread_mutex_unlock(&perfcalc_lock);

		/* calculate router throughput */
		if( router_data > 1024 ) {

			gettimeofday(&now, NULL);	/* get timestamp */

			/* calculate data */
			
			/* the next two lines sould be executed atomically, ups... if not */
			/*			pthread_mutex_lock( &router_data_lock );*/
			data = router_data;
			router_data = 0;
			/*			pthread_mutex_unlock( &router_data_lock );*/

			if( data < 1024 ) {
				router_multi = 1;
			}
			else if( data < 1048576 ) {
				data /= 1024;
				router_multi = 1024;		/* value for multiplikation to get the correct router_load-value, because we have divide a line above */
				/* workaround for the limitation in the unsigned long int range of values */
			}
 			else {
				data /= 1048576;
				router_multi = 1048576;
			}		

			time = (unsigned long int) ( now.tv_sec - last.tv_sec );	/* seconds */
			utime = (unsigned long int) ( now.tv_usec+1000000 - last.tv_usec );	/* microsec */
												   /* ^^^^^^^ so that we never get negative values */
			if( utime >= 1000000 )
					utime -= 1000000;	/* microsec */
			else	time += 1;	/* tv_sec + 1 from tv_usec (carryflag) */
			
			/* performance of the router calculated in Bytes per second , this two lines have to be executed nearly atomically, too */
			/*			pthread_mutex_lock( &router_load_lock );*/
			
			router_help = (unsigned long int) ( (data* 1000000 )/((time*1000000)+utime));
 			router_help = router_help * router_multi;
			router_load = router_help;
			
			/*			pthread_mutex_unlock( &router_load_lock );*/


			gettimeofday(&last, NULL);	/* start timer */

			/* wake all send_threads */
			pthread_mutex_lock( &perfsend_lock );
			pthread_cond_broadcast( &perfsend_cv );
			pthread_mutex_unlock( &perfsend_lock );


		} /* end of if */

	} /* end of while */
			
	PDEBUG( "calc_thread: calculate thread terminates...\n" );					/* DEBUG */
	
	/* destroy thread*/
	pthread_exit( NULL );
}

/* a thread, which exports router performance data via a TCP/IP socket to a remote client */
void *perfsend_thread(void *args ) {
	unsigned long int	quit_message;
	char			state;

	int newsockfd;

	unsigned long int	realdata;									/* DEBUG */
	unsigned long int	time;
	unsigned long int	utime;
	realdata = 2000000;
	time = 1;
	utime = 1;

	newsockfd = *(int *)args;

	state = 'a';		/* for abort signaling */

	/* run until application exits */
	while( perf_app_status == PERF_APP_RUNNING ) {
	    unsigned long int loadTemp;
		/* wait for signal */
		pthread_mutex_lock( &perfsend_lock );
		pthread_cond_wait( &perfsend_cv, &perfsend_lock );
		pthread_mutex_unlock( &perfsend_lock );

		/* transmit data, must be secured by a mutex */
		/*		pthread_mutex_lock( &router_load_lock );*/
		loadTemp=htonl(router_load);
		write( newsockfd, &loadTemp, sizeof(router_load) );
		PDEBUG1( " %i\n", router_load );					/* DEBUG */
		/*		pthread_mutex_unlock( &router_load_lock );*/

		/* react if the client quits */
   		if( recv( newsockfd, &state, sizeof(state), 0 ) < 0 )
			printf( "send_thread: read from client fails\n" );	

		if( state == 'a' ) break; /* exit this sending thread, because the client terminated */
	}

	/* tell the client to quit, if it was not terminated */
	if( state != 'a' ) {
		quit_message = -1;
		write( newsockfd, &quit_message, sizeof(quit_message) );
		PDEBUG( "send_thread: telling the client to quit!\n" );			/* DEBUG */
	}

	close( newsockfd );
	
	PDEBUG( "send_thread: send thread terminates... " );				/* DEBUG */
	
	/* destroy thread */
	pthread_exit( NULL );
}

/* a thread, which waits for connections from clients and establishes connections with them */
void *perfconnect_thread( void *null ) {

	int					val;
	int					len;
	int					i, status, sockfd, newsockfd, clientlen; /* fd = filediscriptor */
	struct sockaddr_in	server_addr, client_addr;
	pthread_t			accept_thread;

	/* list of IDs of the sendingthreads */
	struct sendthread {
		pthread_t			id;
		struct sendthread*	next;
	};

	/* create pointers to a list of thread IDs for the sendthreads */
	struct sendthread *all_threads;
	struct sendthread *this_thread;
	
	struct timeval	timeout;
	fd_set			readfds;
	
	timeout.tv_sec 	= 0;
	timeout.tv_usec = 1000;
	
	/* create a TCP/IP Socket for the perfmonitorserver */
	if( (sockfd = socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
		printf( "Perfserver: can't open stream socket\n" );
	
	/* set socket option: allow local address reuse */
	len = sizeof( val );
	val = 1;
	setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, len );
	
	/* bind the local address so that a client can send to us */
	bzero( (char *) &server_addr, sizeof( server_addr ));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= htonl( INADDR_ANY );
	server_addr.sin_port		= htons( PERFSERV_TCP_PORT );
	
	/* here we get problems if the port is already in use
	 * workaround:
	 * if the Port is not available, increment Portnumber and try again
	 * this enables currently 10 routers on a host, with different Perfmonitors
	 */
	for( i = 1; i < 11; i++ ) {
		status = bind( sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr) );
		
		if( status < 0 )
			server_addr.sin_port = htons( PERFSERV_TCP_PORT + i );
		else {
			printf( "Performance Server is on Port %i\n", PERFSERV_TCP_PORT + i - 1 );
			break;
		}
		
		if( i == 10 ) {
			printf( "Perfserver: can't bind local address. Error: %s\n", strerror(errno) );
		}
	}

	/* enable max. 5 queued connectionrequests with the Performance-Monitor Server */
	listen( sockfd, 5 );

	/* init mutex and condition variables for the sendthreads */
	pthread_cond_init( &perfsend_cv, NULL );
	pthread_mutex_init( &perfsend_lock, NULL );

	/* create a dynamic list of thread IDs for the sendthreads */
	all_threads = (struct sendthread *)malloc( sizeof(struct sendthread) );
	all_threads->id = 0;
	all_threads->next = NULL;
	
	this_thread = all_threads;

	FD_ZERO( &readfds );
      		
	clientlen = sizeof( client_addr );

	/* run until application exits */
	while( perf_app_status == PERF_APP_RUNNING ) {

		/* test for a connection from a client process */
		FD_SET( sockfd, &readfds );
		select( sockfd + 1, &readfds, NULL, NULL, &timeout );
		if( FD_ISSET( sockfd, &readfds ))
		{
			PDEBUG( "conn_thread: incoming request for a connection\n" );	/* DEBUG */
			
			/* establish the connection with the client process */
			newsockfd = accept( sockfd, (struct sockaddr *)&client_addr, &clientlen );

	 		if( newsockfd < 0 )
				printf( "Perfserver: accept error\n" );
			else {
				/* when a client connects, start a thread,
				 * which will send the perfmeter client performance related data
				 */
				pthread_create( &this_thread->id, NULL, perfsend_thread, (void *)&newsockfd );
				
				/* create new entry in the sendthread ID list*/		
				this_thread->next = (struct sendthread *)malloc( sizeof(struct sendthread) );
				this_thread		  = this_thread->next;
				this_thread->id = 0;
				this_thread->next = NULL;
			}
		
			FD_CLR( sockfd, &readfds );
		}
	}

	PDEBUG( "conn_thread: starting with shutdown\n" );				/* DEBUG */

	/* close the original socket */
	close( sockfd );
		
	/* wake all send_threads in order to terminate them */
	pthread_mutex_lock( &perfsend_lock );
	pthread_cond_broadcast( &perfsend_cv );
	pthread_mutex_unlock( &perfsend_lock );
	
	PDEBUG( "conn_thread: start to wait for the sendthread termination... " );	/* DEBUG */
	
	/* here we must wait for termination of all perfsend_threads,
	 * so that all childsockets are closed
	 * and we must free the list
	 */
	while( all_threads != NULL ) {
		if( all_threads->id != 0 ) pthread_join( all_threads->id, NULL );
		this_thread = all_threads;		
		all_threads = all_threads->next;
		free( this_thread );
	}
	
	PDEBUG( "Done!\n" );								/* DEBUG */
	
	PDEBUG( "conn_thread: connection thread terminates...\n" );			/* DEBUG */

	/* destroy thread */
	pthread_exit( NULL );
}

/* the basic init funktion for the performance-Monitor */
int init_perfmeter() {

	PDEBUG( "init: Perfserver initialisation started... " );			/* DEBUG */

	router_data = 0;   	/* init countvariable */
	router_load = 0;

	perf_app_status = PERF_APP_RUNNING;
	
	/*	pthread_mutex_init( &router_data_lock, NULL );*/
	/*	pthread_mutex_init( &router_load_lock, NULL );*/


	/* init some vars for the perf_thread signaling and syncronisation */
	pthread_cond_init( &perfcalc_cv, NULL );
	pthread_mutex_init( &perfcalc_lock, NULL );

	/* create a thread, which measures the router throughput and nothing other */
	pthread_create( &perfcalc_thread, NULL, perfcalculate_thread, NULL );

	/* create a thread, which waits for connections from clients,
	 * for each connection a thread is created and within this threads the
	 * measured throughput data is send to the clients
 	 */
	pthread_create( &perfconn_thread, NULL, perfconnect_thread, NULL );

 	/* now the main process can continue to execute */
	return(0);
}

/* destroy all structures used by the perfserver and exit it */
int terminate_perfmeter()
{
	PDEBUG( "exit: start quitting\n" );						/* DEBUG */

	perf_app_status = PERF_APP_FINALIZE;	/* tell everyone to end */

	/* wake perfcalculate_thread in order to terminate it */
	pthread_mutex_lock( &perfcalc_lock );
	pthread_cond_signal( &perfcalc_cv );
	pthread_mutex_unlock( &perfcalc_lock );

	/* wait for the threads to end */
	
	PDEBUG( "exit: waiting for termination of the perfcalc thread...\n" );		/* DEBUG */
	
	pthread_join( perfcalc_thread, NULL );
	pthread_join( perfconn_thread, NULL );

	PDEBUG( "exit: Done!\n" );							/* DEBUG */

	return(0);
}

#endif
