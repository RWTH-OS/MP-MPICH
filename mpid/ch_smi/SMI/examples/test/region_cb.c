/* $Id$
  
   Test callbacks for shared memory regions.

*/

#include <stdio.h>
#include <sys/types.h>

#include "smi.h"


#define REGION_SIZE (1024*1024)
#define SRC_RANK  1
#define DST_RANK  0
#define DELAY     3


static int callback_fcn (int region_id, int cb_reason)
{
    printf ("Callback for region %d, reason is %d.\n", region_id, cb_reason);

    return SMI_CB_ACTION_FREEREG;
}


int main (int argc, char *argv[]) 
{
    smi_region_info_t smi_region;
    int nprocs, myrank, shreg_id, sgmt_id;
    char *src_adr, *dst_adr;

    SMI_Init (&argc, &argv);
    SMI_Proc_size(&nprocs);
    SMI_Proc_rank(&myrank);

    if (nprocs != 2) {
	fprintf(stderr, "this test may only be run with 2 processes - aborting.\n");
	SMI_Finalize();
	return (0);
    }
    
    if (myrank == SRC_RANK) {
	/* Create a region, let the other process connect, then wait for 5 seconds
	   and remove the region. */
	printf ("[%d] Creating local shared region.\n", myrank);
	SMI_Init_reginfo(&smi_region, REGION_SIZE, 0, myrank, SMI_ADPT_DEFAULT, 0, 0, NULL);
	SMI_Create_shreg(SMI_SHM_LOCAL, &smi_region, &shreg_id, (void **)&src_adr);

	SMI_Query (SMI_Q_SMI_REGION_SGMT_ID, shreg_id, &sgmt_id);
	SMI_Send (&sgmt_id, sizeof(int), DST_RANK);

	printf ("[%d] Sleeping for %d seconds.\n", myrank, DELAY);
	sleep (DELAY);

	printf ("[%d] Freeing local shared region.\n", myrank);
	SMI_Free_shreg (shreg_id);
    } else {
	SMI_Recv (&sgmt_id,  sizeof(int), SRC_RANK);
	
	printf ("[%d] Creating remote shared region.\n", myrank);
	SMI_Init_reginfo(&smi_region, 0, 0, SRC_RANK, SMI_ADPT_DEFAULT, 0, sgmt_id, callback_fcn);
	if (SMI_Create_shreg(SMI_SHM_REMOTE|SMI_SHM_CALLBACK, &smi_region, &shreg_id, (void **)&dst_adr)
	    == SMI_SUCCESS)
	    printf ("[%d] Created remote shared region with id %d.\n", myrank, shreg_id);
	else {
	    printf ("[%d] ERROR: could not create remote region!\n", myrank);
	    SMI_Abort (-1);
	}
	
	printf ("[%d] Sleeping for %d seconds.\n", myrank, 2*DELAY);
	sleep (2*DELAY);
	
	if (SMI_Adr_to_region (dst_adr, &shreg_id) == SMI_SUCCESS)
	    printf ("[%d] ERROR: remote shared region still exists!.\n", myrank);
	else
	    printf ("[%d] Success: remote shared region was freed.\n", myrank);
    }
	
    SMI_Finalize();
    return 0;
}

