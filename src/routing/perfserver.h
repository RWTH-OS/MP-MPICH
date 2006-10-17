/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
/*
 * perfserver.h
 *
 * Definition of the Performance-Monitor Server
 *
 */

#ifndef PERFSERVER_H
#define PERFSERVER_H

/* debugging messages */
/*#ifndef PERFSERVER_DEBUG
#define PERFSERVER_DEBUG
#endif
*/


#ifdef PERFSERVER_DEBUG
#define PDEBUG(a) fprintf(stderr, a); fflush(NULL);
#define PDEBUG1(a,b) fprintf(stderr, a, b); fflush(NULL);
#else
#define PDEBUG(a) 
#define PDEBUG1(a,b) 
#endif

/* global variables for the performance-Monitor Server */
extern pthread_mutex_t	perfcalc_lock;
extern pthread_cond_t	perfcalc_cv;
extern int		router_data; /* used in perfcalculate_thread() and in export_msgs() (in mpi_router.c) */

/*extern pthread_mutex_t router_data_lock;*/

/* perfserver exports these funktions: */

/* the basic init funktion for the performance-Monitor */
int init_perfmeter();

/* destroy all structures used by the perfserver and exit it */
int terminate_perfmeter();

/* wake perfserver in order to send performance related data to the clients */
/*#define WAKE_PERFSERVER	pthread_mutex_lock( &perfcalc_lock ); pthread_cond_signal( &perfcalc_cv ); pthread_mutex_unlock( &perfcalc_lock );*/

#define WAKE_PERFSERVER 
/* add amount of data to the router_data variable, this is performance related data */
/*#define SENDTO_PERFSERVER(a) pthread_mutex_lock( &router_data_lock ); \
                             router_data += a; 	   \
                             pthread_mutex_lock( &router_data_lock );

*/
#define SENDTO_PERFSERVER(a) router_data += a; 	  
#endif

