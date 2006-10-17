/*$Id$*/
#include <stdio.h>
#include "mpi.h"

#ifndef WIN32
#include <unistd.h>
#endif

/* 
#define PRINT((A)) fprintf (stderr, "[%d] ", w_id); fprintf (stderr, (A)); fprintf (stderr, "\n");
*/
#define BACKOFF_TIME_US 1000

int main(argc,argv)
int argc;
char *argv[];
{
    int d_rank;
    int w_np, w_id, h_np, h_id, l_np, l_id;
    int  namelen,metahostnamelen;

    int dummy = 1;
    int tag, flag, pass;

    double msg = 1.234567;
    double ref = 1.234567;

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    char metahost_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Status status;

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&w_np);
    MPI_Comm_rank(MPI_COMM_WORLD,&w_id);

    MPI_Comm_size(MPI_COMM_HOST,&h_np);
    MPI_Comm_rank(MPI_COMM_HOST,&h_id);
    
    MPI_Comm_size(MPI_COMM_LOCAL,&l_np);
    MPI_Comm_rank(MPI_COMM_LOCAL,&l_id);
    
    MPI_Attr_get(MPI_COMM_WORLD, MPI_TAG_UB, &tag, &flag);
    if (flag)
	fprintf (stderr, "Upper TAG bound = %d\n", tag);

    MPI_Get_processor_name(processor_name,&namelen);
    MPI_META_Get_metahost_name(metahost_name,&metahostnamelen);

    fprintf(stderr, "Meta Host: %s: Process %d on %s\n w_np = %d w_id = %d h_np = %d h_id = %d l_np = %d l_id = %d\n",metahost_name, 
	    w_id, processor_name, w_np, w_id, h_np, h_id, l_np, l_id);    

    /* 
     *  do some meta - testing: master 0 sends msgs to all, then all reply to master
     */

    /* test local communicator */
    fprintf (stderr, "[%d] Testing COMM_LOCAL\n", l_id);

    /* 0 sends to all with indiviual sends, they reply back to 0 */
    if (l_id == 0) {
	for (d_rank = 1; d_rank < l_np; d_rank++) {
	    fprintf (stderr, "[%d] sending number = %f to %d\n", l_id, msg, d_rank);
	    MPI_Send ((void *)&msg, 1, MPI_DOUBLE, d_rank, 0, MPI_COMM_LOCAL);
	}

	fprintf (stderr, "[%d] waiting...\n", l_id);

	for (d_rank = 1; d_rank < l_np; d_rank++) {
	    MPI_Recv ((void *)&msg, 1, MPI_DOUBLE, d_rank, 0, MPI_COMM_LOCAL, &status);
	    if (msg == ref)
		fprintf (stderr, "[%d] MPI_recv from [%d] O.K.\n", l_id, d_rank);
	    else
		fprintf (stderr, "[%d] *** MPI_recv from [%d] failed: expected %f, got %f\n", 
			 l_id, d_rank, ref, msg);		
	}
    } else {
	fprintf (stderr, "[%d] receiving...\n", l_id);

	MPI_Recv ((void *)&msg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_LOCAL, &status);
	if (msg == ref)
	    fprintf (stderr, "[%d] MPI_recv from 0 O.K.\n", l_id);
	else
	    fprintf (stderr, "[%d] *** MPI_recv from 0 failed: expected %f, got %f\n", 
		     l_id, ref, msg);

	fprintf (stderr, "[%d] sending number = %f to 0", l_id, msg);
	MPI_Send ((void *)&msg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_LOCAL);

    }
    
    /* broadcast from 0 
    MPI_Bcast ((void *)&msg, 1, MPI_DOUBLE, 0, MPI_COMM_LOCAL);

    if (msg == ref) 
	fprintf (stderr, "[%d] Broadcast from 0 O.K.\n", l_id);
    else
	fprintf (stderr, "[%d] *** Broadcast from 0 failed: expected %f, got %f\n", 
		     l_id, ref, msg);
		     */
    fprintf (stderr, "[%d] Barrier synchronization.\n", l_id);    
    MPI_Barrier (MPI_COMM_LOCAL);
    
    fprintf (stderr, "[%d] COMM_LOCAL test o.k.\n", l_id);

    /*
     * test world communicator 
     */

    msg = 1.234567;
    fprintf (stderr, "[%d] Testing COMM_WORLD\n", w_id);

    if (w_id == 0) {
	for (d_rank = 1; d_rank < w_np; d_rank++) {
	    fprintf (stderr, "[%d] sending number = %f to %d\n", w_id, msg, d_rank);
	    MPI_Send ((void *)&msg, 1, MPI_DOUBLE, d_rank, 0, MPI_COMM_WORLD);
	}

	fprintf (stderr, "[%d] waiting...\n", w_id);

	for (d_rank = 1; d_rank < w_np; d_rank++) {
	    MPI_Recv ((void *)&msg, 1, MPI_DOUBLE, d_rank, 0, MPI_COMM_WORLD, &status);
	    if (msg == ref)
		fprintf (stderr, "[%d] MPI_recv from [%d] O.K.\n", w_id, d_rank);
	    else
		fprintf (stderr, "[%d] *** MPI_recv from [%d] failed: expected %f, got %f\n", 
			 w_id, d_rank, ref, msg);		
	}
    } else {
	fprintf (stderr, "[%d] receiving..\n", w_id);

	MPI_Recv ((void *)&msg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
	if (msg == ref)
	    fprintf (stderr, "[%d] MPI_recv from 0 O.K.\n", w_id);
	else
	    fprintf (stderr, "[%d] *** MPI_recv from 0 failed: expected %f, got %f\n", 
		     w_id, ref, msg);

	fprintf (stderr, "[%d] sending number = %f to 0\n", w_id, msg);
	MPI_Send ((void *)&msg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

    }

    /* broadcast from 0 
    MPI_Bcast ((void *)&msg, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (msg == ref)
	fprintf (stderr, "[%d] Broadcast from 0 O.K.\n", w_id);
    else
	fprintf (stderr, "[%d] *** Broadcast from 0 failed: expected %f, got %f\n", 
		     w_id, ref, msg);
		     */

    fprintf (stderr, "[%d] Barrier synchronization.\n", w_id);    
    MPI_Barrier (MPI_COMM_WORLD);
    
    fprintf (stderr, "[%d] COMM_WORLD test o.k.\n", w_id);
    
    MPI_Finalize();
    fprintf(stderr,"MPI_Finalize successful!\n");
    return 0;
}

            
