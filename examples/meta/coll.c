#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

/* test of collective communications */

int main(argc,argv)
int argc;
char *argv[];
{
    int d_rank;
    int w_np, w_id, h_np, h_id, l_np, l_id;
    int  namelen;
    int i, j;

    int msg = 1234567;
    int ref = 1234567;
    int flag = 0;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Status send_stat[5];
    MPI_Status recv_stat[5];
    MPI_Request send_req[5];
    MPI_Request recv_req[5];

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&w_np);
    MPI_Comm_rank(MPI_COMM_WORLD,&w_id);

    MPI_Comm_size(MPI_COMM_HOST,&h_np);
    MPI_Comm_rank(MPI_COMM_HOST,&h_id);
    
    MPI_Comm_size(MPI_COMM_LOCAL,&l_np);
    MPI_Comm_rank(MPI_COMM_LOCAL,&l_id);
    
    MPI_Get_processor_name(processor_name,&namelen);
    fprintf(stderr, "Process %d on %s\n w_np = %d w_id = %d h_np = %d h_id = %d l_np = %d l_id = %d\n", 
	    w_id, processor_name, w_np, w_id, h_np, h_id, l_np, l_id);    

    /* test local communicator 
    fprintf (stderr, "[%d] Testing COMM_LOCAL\n", l_id);
    */
    /* broadcast from 0 
    MPI_Bcast ((void *)&msg, 1, MPI_DOUBLE, 0, MPI_COMM_LOCAL);

    if (msg == ref) 
	fprintf (stderr, "[%d] Broadcast from 0 O.K.\n", l_id);
    else
	fprintf (stderr, "[%d] *** Broadcast from 0 failed: expected %f, got %f\n", 
		     l_id, ref, msg);
		     */
    /* 
    fprintf (stderr, "[%d] Barrier synchronization.\n", l_id);    
    MPI_Barrier (MPI_COMM_LOCAL);
    
    fprintf (stderr, "[%d] COMM_LOCAL test o.k.\n", l_id);
    */

    /*
     * test world communicator 
     */

    fprintf (stderr, "[%d] Testing COMM_WORLD\n", w_id);

    /* send & recv a normal integer */
    fprintf (stderr, "\n[%d] Testing normal Isend/Irecv\n", w_id);
    if (w_id == 0) {
	for (d_rank = 1; d_rank < w_np; d_rank++) {
	    fprintf (stderr, "[%d] sending int number = %d to %d\n", w_id, msg, d_rank);
	    MPI_Isend ((void *)&msg, 1, MPI_INT, d_rank, 0, MPI_COMM_WORLD, &send_req[d_rank]);
	}
	fprintf (stderr, "[%d] testing Isend\n", w_id);
	for (d_rank = 1; d_rank < w_np; d_rank++) 
	    do {
		MPI_Test (&send_req[d_rank], &flag, &send_stat[d_rank]);
		/*	    fprintf (stderr, ".");*/
	    } while (!flag);

	for (d_rank = 1; d_rank < w_np; d_rank++) {
	    MPI_Irecv ((void *)&msg, 1, MPI_INT, d_rank, 0, MPI_COMM_WORLD, &recv_req[d_rank]);
	    fprintf (stderr, "[%d] waiting for Irecv from [%d]\n", w_id, d_rank);
	    do {
		MPI_Test (&recv_req[d_rank], &flag, &recv_stat[d_rank]);
		/*		fprintf (stderr, ".");*/
	    } while (!flag);

	    if (msg == ref)
		fprintf (stderr, "[%d] MPI_Irecv from [%d] O.K.\n", w_id, d_rank);
	    else
		fprintf (stderr, "[%d] *** MPI_Irecv from [%d] failed: expected %d, got %d\n", 
			 w_id, d_rank, ref, msg);		
	}
    } else {
	fprintf (stderr, "[%d] receiving int ..\n", w_id);

	MPI_Irecv ((void *)&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &recv_req[w_id]);
	fprintf (stderr, "[%d] waiting for Irecv from [0]\n", w_id);
	do {
	    MPI_Test (&recv_req[w_id], &flag, &recv_stat[w_id]);
	    /*	    fprintf (stderr, ".");*/
	} while (!flag);

	if (msg == ref)
	    fprintf (stderr, "[%d] MPI_Irecv from 0 O.K.\n", w_id);
	else
	    fprintf (stderr, "[%d] *** MPI_Irecv from 0 failed: expected %d, got %d\n", 
		     w_id, ref, msg);

	fprintf (stderr, "[%d] sending number = %d to 0\n", w_id, msg);
	MPI_Isend ((void *)&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &send_req[w_id]);
	fprintf (stderr, "[%d] testing\n", w_id);
	do {
	    MPI_Test (&send_req[w_id], &flag, &send_stat[w_id]);
	    /*	    fprintf (stderr, ".");*/
	} while (!flag);
    }

    /* send & recv an "empty" integer like it ist done in intra_barrier() 
    fprintf (stderr, "\n[%d] Testing empty Isend/Irecv\n", w_id);
    if (w_id == 0) {
	for (d_rank = 1; d_rank < w_np; d_rank++) {
	    fprintf (stderr, "[%d] sending empty int to %d\n", w_id, d_rank);
	    MPI_Isend ((void *)0, 0, MPI_INT, d_rank, 1, MPI_COMM_WORLD, &send_req);
	}
	fprintf (stderr, "[%d] waiting", w_id);
	do {
	    MPI_Test (&send_req, &flag, &send_stat);
	    fprintf (stderr, ".");
	} while (!flag);
	fprintf (stderr, "\n");

	for (d_rank = 1; d_rank < w_np; d_rank++) {
	    MPI_Irecv ((void *)0, 0, MPI_INT, d_rank, 1, MPI_COMM_WORLD, &recv_req);
	    fprintf (stderr, "[%d] waiting for Irecv", w_id);
	    do {
		MPI_Test (&recv_req, &flag, &recv_stat);
		fprintf (stderr, ".");
	    } while (!flag);
	    fprintf (stderr, "\n");

	    if (msg == ref)
		fprintf (stderr, "[%d] empty MPI_Irecv from [%d] O.K.\n", w_id, d_rank);
	    else
		fprintf (stderr, "[%d] *** empty MPI_Irecv from [%d] failed\n", w_id, d_rank);		
	}
    } else {
	fprintf (stderr, "[%d] receiving int ..\n", w_id);

	MPI_Irecv ((void *)0, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &recv_req);
	fprintf (stderr, "[%d] waiting for Irecv", w_id);
	do {
	    MPI_Test (&recv_req, &flag, &recv_stat);
	    fprintf (stderr, ".");
	} while (!flag);
	fprintf (stderr, "\n");

	if (msg == ref)
	    fprintf (stderr, "[%d] empty MPI_Irecv from 0 O.K.\n", w_id);
	else
	    fprintf (stderr, "[%d] *** empty MPI_Irecv from 0 failed\n", w_id);

	fprintf (stderr, "[%d] sending empty int to 0\n", w_id);
	MPI_Isend ((void *)0, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &send_req);
	fprintf (stderr, "[%d] waiting", w_id);
	do {
	    MPI_Test (&send_req, &flag, &send_stat);
	    fprintf (stderr, ".");
	} while (!flag);
	fprintf (stderr, "\n");
    }
    */
    
    /* test SendRecv combination 
    fprintf (stderr, "\n[%d] Testing empty Sendrecv\n", w_id);
    for (j = 0; j < 20; j++)
	for (i = 0; i < w_np; i++)
	    if (i != w_id)
		MPI_Sendrecv( (void *)0,0,MPI_INT, i, 1,
			      (void *)0,0,MPI_INT, i, 1, 
			      MPI_COMM_WORLD, &send_stat);
			      */
    
    /* broadcast from 0 */
    fprintf (stderr, "[%d] Broadcast from [0].\n", w_id);    
    MPI_Bcast ((void *)&msg, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (msg == ref)
	fprintf (stderr, "[%d] Broadcast from [0] O.K.\n", w_id);
    else
	fprintf (stderr, "[%d] *** Broadcast from [0] failed: expected %d, got %d\n", 
		     w_id, ref, msg);
		     

    fprintf (stderr, "[%d] Barrier synchronization.\n", w_id);    
    MPI_Barrier (MPI_COMM_WORLD);
    
    fprintf (stderr, "[%d] COMM_WORLD test o.k.\n", w_id);
    
    MPI_Finalize();
    return 0;
}

            
