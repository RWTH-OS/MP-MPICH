/* $Id$ */
#ifndef _MPI_TIMERWRAP_H
#define _MPI_TIMERWRAP_H

/* datastructures for statistical MPI wrapper */

typedef enum {
    application,	/* time for the application */
    collective_ops,	/* time for collective operations */
    barrier,		/* time spent in barriers */
    send_blck,		/* time spent in blocking sends */
    send_noblck,	/* " nonblocking sends */
    recv_blck,		/* time spent in blocking receives */
    recv_noblck,	/* " nonblocking receives */
    wait,		/* wait, test, cancel */
    onesided_put,       
    onesided_get,
    onesided_accu,
    onesided_sync,
    memory,
    def,		/* other MPI-functions */
        
    fun_dummy		/*** stop marker ***/
} function_number_t;

typedef struct {
    char *name;                         /* name of function group */
    int number_calls;                   /* how many times called */
    double min_time;                    /* minimal time spent */
    double max_time;                    /* maximal time spent */
    double acc_time;                    /* accumulated time */
} statistics_t;

#endif
