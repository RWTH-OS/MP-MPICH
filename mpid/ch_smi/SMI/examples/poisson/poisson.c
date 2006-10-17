/* $Id$ 
   grid-based solver for 2D poisson equation 

   This programm requires the full SMI library! 
   (libsmi instead of libcsmi, invoke configure with the --enable-full_smi option)
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#include <smi.h>

#define GRIDSIZE   256		/* size of the discretization grid   */
#define ITERATIONS 5000	/* number of iterations to perform   */

int main(int argc, char **argv)
{
    smi_region_info_t regdesc;
    smi_error_t         error;	/* return code */
    int             shseg_id;	/* identifier of the shared memory segment   */
    double         *grid;	/* GRIDSIZE x GRIDZISE discretization array  */
    int             i, j, t;    /* loop counter variables                    */
    int             proc_rank;	/* rank of this process within all processes */
    int             proc_size;	/* total number of processes                 */
    int             loop_id;    /* identifier of the loop that is split      */
    int             low, high;	/* lower and upper loop bound of the split loop   */
    double  t1, t2;	        /* for timing purposes                       */
    int             status;	/* returned status of the scheduling	     */


    error = SMI_Init(&argc, &argv);
    if (error != SMI_SUCCESS) {
	fprintf(stderr, "proc=%i location=0 error=%i\n", proc_rank, error);
	SMI_Abort(-1);
    }
    SMI_Proc_rank(&proc_rank);
    SMI_Proc_size(&proc_size);

    /* setup a shared memory region */
    SMI_Init_reginfo (&regdesc, GRIDSIZE * GRIDSIZE * sizeof(double), 0 ,0,0,0,0,0);
    error = SMI_Create_shreg(SMI_SHM_BLOCKED, &regdesc, &shseg_id, (char **) &grid);
    if (error != SMI_SUCCESS) {
	fprintf(stderr, "[%d] Could not create shared memory region, SMI error %d\n", proc_rank, error);
	SMI_Abort(-1);
    }

    /* the loop splitting */
    low = 0;
    high = GRIDSIZE - 1;
    SMI_Loop_init(&loop_id, low, high, SMI_PART_BLOCKED);
    SMI_Set_loop_help_param(loop_id, 0);
    SMI_Set_loop_param(loop_id, SMI_NO_CHANGE, 10000, 10000, 10000, 10000);

    do {
	SMI_Get_iterations(loop_id, &status, &low, &high);
	if (low == 0)
	    low = 1;
	if (high == GRIDSIZE - 1)
	    high = GRIDSIZE - 2;

	/* intialize the grid */
	for (i = low; i <= high; i++)
	    for (j = 0; j < GRIDSIZE; j++)
		if (i == 0 || j == 0 || i == GRIDSIZE - 1 || j == GRIDSIZE - 1)
		    grid[i * GRIDSIZE + j] = 0.5 + 
			0.5* sin(8.0*(double)i/(double)GRIDSIZE + 8.0*(double)j/(double)GRIDSIZE);
		else
		    grid[i * GRIDSIZE + j] = 0.0;
    } while (status != SMI_LOOP_READY);
    
    /* the graphics */
    /* not inlcluded */
    
    /* calculate */
    SMI_Barrier();
    t1 = SMI_Wtime();
    for (t = 0; t < ITERATIONS; t++) {
	do {
	    SMI_Get_iterations(loop_id, &status, &low, &high);
	    if (low == 0)
		low = 1;
	    if (high == GRIDSIZE - 1)
		high = GRIDSIZE - 2;

	    /* calculation */
	    for (i = low; i <= high; i++)
		for (j = 1; j < GRIDSIZE - 1; j++)
		    grid[i * GRIDSIZE + j] = 0.25 * (grid[i * GRIDSIZE + j + 1]
						     + grid[i * GRIDSIZE + j - 1]
						     + grid[(i + 1) * GRIDSIZE + j]
						     + grid[(i - 1) * GRIDSIZE + j]);
	} while (status != SMI_LOOP_READY);

	if (t % 100 == 0 && proc_rank == 0) {
	    t2 = SMI_Wtime();
	    fprintf(stderr, "time for 100 iterations = %.6f s\n", t2-t1);
	    t1 = SMI_Wtime();
	}
    }
    SMI_Barrier();

    /* finalize */
    SMI_Loop_free(loop_id);
    SMI_Finalize();

    return 0;
}
