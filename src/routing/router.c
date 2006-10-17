/* $Id$ 
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 */


#include "mpi_router.h"

int main(int argc, char **argv){
#ifdef WIN32
	SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
#endif
	fprintf(stderr,"running dedicated router process ...\n");
	MPI_Init(&argc, &argv);
    	/* MPI_Finalize() is in MPIR_Router */
	fprintf(stderr,"dedicated router process exited ...\n");
	return 0;
}
