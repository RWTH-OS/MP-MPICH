/* $Id$
  
   Test DMA performance and capabilities.

*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "smi.h"


#define DMA_SIZE (1024*1024)
#define SRC_RANK  1
#define DST_RANK  0


int main (int argc, char *argv[]) 
{
    smi_memcpy_handle mc_h = NULL;
    smi_region_info_t smi_region;
    int nprocs, myrank, src_shreg_id, dst_shreg_id;
    char *src_adr, *dst_adr;

    SMI_Init (&argc, &argv);
    SMI_Proc_size(&nprocs);
    SMI_Proc_rank(&myrank);

    if (nprocs != 2) {
	fprintf(stderr, "this test may only be run with 2 processes - aborting.\n");
	SMI_Finalize();
	return (0);
    }

    /* create src region */
    if (myrank == SRC_RANK) {
	smi_region.size   = DMA_SIZE;
	smi_region.owner  = SRC_RANK;
	smi_region.offset = 0;
	smi_region.adapter = SMI_ADPT_DEFAULT;

	SMI_Create_shreg(SMI_SHM_LOCAL, &smi_region, &src_shreg_id, (void **)&src_adr);
	strcpy(src_adr, "Dies ist der Text vom Sender");
    }

    /* create dst region */
    smi_region.size   = DMA_SIZE;
    smi_region.owner  = DST_RANK;
    smi_region.offset = 0;
    smi_region.adapter = SMI_ADPT_DEFAULT;
    
    SMI_Create_shreg(SMI_SHM_UNDIVIDED, &smi_region, &dst_shreg_id, (void **)&dst_adr);
    
    /*
     * simple DMA send & wait test 
     */
    if (myrank == SRC_RANK) {
	printf ("Posting DMA transfer...\n"); fflush (stdout);
	/* move data */
	SMI_Imemcpy (dst_adr, src_adr, DMA_SIZE, SMI_MEMCPY_LS_RS, &mc_h);
	printf ("Waiting for completion of DMA transfer..\n"); fflush (stdout);
	SMI_Memwait (mc_h);
	printf ("DMA transfer finished.\n"); fflush (stdout);
	
	SMI_Signal_send(DST_RANK|SMI_SIGNAL_ANY);
	
    } else {
	SMI_Signal_wait(SMI_SIGNAL_ANY);
	printf("got message: \"%s\" \n",dst_adr);
    }
    
    /*
     * multiple enqueiung of DMA, post & wait test 
     */

    /*
     * DMA with callback 
     */    

    /* clean up */
    if (myrank == SRC_RANK)
      SMI_Free_shreg (src_shreg_id);
    SMI_Free_shreg (dst_shreg_id);
    SMI_Finalize();
    return 0;
}

