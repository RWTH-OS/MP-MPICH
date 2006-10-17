/* */

#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

int main(argc,argv)
int argc;
char *argv[];
{
    int d_rank;
    int w_np, w_id, l_np, l_id;
    int  namelen;

    int dummy = 1;
    int tag, flag, pass;

    double msg = 1.234567;
    double ref = 1.234567;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Status status;

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&w_np);
    MPI_Comm_rank(MPI_COMM_WORLD,&w_id);

    MPI_Comm_size(MPI_COMM_LOCAL,&l_np);
    MPI_Comm_rank(MPI_COMM_LOCAL,&l_id);

    printf ("W_np %d  W_id %d / L_np %d L_id %d\n", w_np, w_id, l_np, l_id);
    
    /* until now, only two processes are supported (one on each host) */
    if (w_np != 2) {
	printf ("hetero needs exactly two processes! \n");
	exit (MPI_Finalize());
    }
    
     if (w_id == 0) {
	 /* send from 0 to 1 */
	 msg = ref;
	MPI_Send (&msg, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);

	MPI_Recv (&msg, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &status);
	if (msg != ref) 
	    printf ("[%d] ERROR: got %f, should be %f \n", w_id, msg, ref);
    } else {
	/* send from 1 to 0 */
	MPI_Recv (&msg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
	if (msg != ref) 
	    printf ("[%d] ERROR: got %f, should be %f \n", w_id, msg, ref);

	msg = ref;
	MPI_Send (&msg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
	
    printf ("[%d] Test completed\n", w_id);
    return (MPI_Finalize());
}

    
    
