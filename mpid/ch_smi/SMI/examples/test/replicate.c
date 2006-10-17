/* $Id: replicate.c,v 1.1 2004/03/19 22:14:46 joachim Exp $ */
/* replicate.c - test the replication and re-shareding of shared memory regions */

#include <stdio.h>

#include "smi.h"


#define SHREG_SIZE (128*1024)

int main (int argc, char *argv[]) 
{
  int rank, size;
  smi_region_info_t reginfo;
  int shreg_id;
  int *shreg_addr;
  int i;
  
  SMI_Init(&argc, &argv);
  
  SMI_Proc_rank(&rank);
  SMI_Proc_size(&size);
  
  SMI_Init_reginfo (&reginfo, SHREG_SIZE, 0);
  SMI_Create_shreg (SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED, &reginfo, &shreg_id, (char **)&shreg_addr);
  
  if (rank == 0) {
    for(i = 0; i < SHREG_SIZE/sizeof(int); i++)
      shreg_addr[i] = 0;
  }
  
  SMI_Barrier();
  
  SMI_Switch_to_replication(shreg_id, SMI_REP_NOTHING,0,0,0);
  
  for(i = 0; i < SHREG_SIZE/sizeof(int); i++)
    shreg_addr[i] = rank;
  
  SMI_Switch_to_sharing(shreg_id, SMI_SHR_SINGLE_SOURCE,/*SHREG_SIZE/sizeof(int)*/1,0);
  
  /* Ueberpruefe ob alle Elemente der Summe aller Prozessraenge entspricht */
  if (rank == 0)
    for(i = 0; i < SHREG_SIZE/sizeof(int); i++)
      if (shreg_addr[i] != (size/2)*(size-1)) {
	fprintf(stderr, "* reduction method add failed! (i:%d %d-%d) *\n",i,shreg_addr[i],(size/2)*(size-1));
	fflush(stderr);
	SMI_Abort(-1);
      }
  
  SMI_Finalize();
}
