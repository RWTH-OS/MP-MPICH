

/* SymbolFakes for smpi.lib*/

#include <stdlib.h>

#include "mpi.h"


#define errorcode -1


int 
PMPI_Win_complete (win)
	MPI_Win win;
{
	return(errorcode);
}

int 
PMPI_Alloc_mem (size, info, baseptr)
	MPI_Aint 	size;
	MPI_Info 	info;
	void 		* baseptr;		/* Keep in mind, that this is a void** 
									using a void** would cause casting
									problems, if the user says &a and a is  
									not a void* but an int*
								 */
{
		return(errorcode);
}


int PMPI_Free_mem (base)
	void * base;
{
		return(errorcode);
}


int PMPI_Win_create( addr, size, rank, info, comm, win )
void * addr;
MPI_Aint size;
int rank;
MPI_Info info;
MPI_Comm comm;
MPI_Win * win;
{
  return(errorcode);
}

int PMPI_Win_free( win )
MPI_Win * win;
{
  return(errorcode);
}

int PMPI_Win_get_attr( win, key, attr, nbr_values )
MPI_Win win;
int key;
void * attr;
int * nbr_values;
{
  return(errorcode);
}

int PMPI_Win_get_group( win, group )
MPI_Win win;
MPI_Group * group;
{
  return(errorcode);
}

int PMPI_Put( addr, count, origin_dtype, rank, displcmnt, target_count, target_dtype, win )
void * addr;
int count;
MPI_Datatype origin_dtype;
int rank;
MPI_Aint displcmnt;
int target_count;
MPI_Datatype target_dtype;
MPI_Win win;
{
  return(errorcode);
}

int PMPI_Get( addr, count, origin_dtype, rank, displcmnt, target_count, target_dtype, win )
void * addr;
int count;
MPI_Datatype origin_dtype;
int rank;
MPI_Aint displcmnt;
int target_count;
MPI_Datatype target_dtype;
MPI_Win win;
{
  return(errorcode);
}

int PMPI_Accumulate( addr, scount, sdtype, rank, displcmnt, target_count, taget_dtype, op, win )
void * addr;
int scount;
MPI_Datatype sdtype;
int rank;
MPI_Aint displcmnt;
int target_count;
MPI_Datatype taget_dtype;
MPI_Op op;
MPI_Win win;
{
  return(errorcode);
}



int PMPI_Win_fence( assert, win )
int  assert;
MPI_Win win;
{
  return(errorcode);
}

int PMPI_Win_start( group, assert, win )
MPI_Group group;
int assert;
MPI_Win win;
{
  return(errorcode);
}


int PMPI_Win_post( group, assert, win )
MPI_Group group;
int assert;
MPI_Win win;
{
  	return(errorcode);
}

int PMPI_Win_wait( win )
MPI_Win win;
{
  	return(errorcode);
}

int PMPI_Win_test( win, flag )
MPI_Win win;
int * flag;
{
  	return(errorcode);
}

int PMPI_Win_lock( locktype, rank, assert, win )
int locktype;
int rank;
int assert;
MPI_Win win;
{
  	return(errorcode);
}

int PMPI_Win_unlock( rank, win )
{
  	return(errorcode);
}
