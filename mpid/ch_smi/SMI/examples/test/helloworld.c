/* $Id$ */
/* helloworld.c - basic SMI startup & shutdown */

#include <stdio.h>
#include <stdlib.h>

#include "smi.h"

#define NAMELEN 128

int main (int argc, char *argv[]) 
{
    int rank, size;
    char name[NAMELEN];
    int pid;
    size_t len = NAMELEN;
    
    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);
    SMI_Get_node_name(name, &len);

    SMI_Query(SMI_Q_SYS_PID, rank, &pid);

    printf ("Hello World - this is process %d of %d, running on %s (pid: %d)\n", rank, size, name,pid);

    SMI_Finalize();
}
