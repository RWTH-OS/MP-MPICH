/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "test.h"

#define BLOCKSIZE 16

int main(int argc, char **argv)
{
	int rank, size;
	int i, j, k;
	int **table, *buffer;
	int begin_row, end_row, send_count, recv_count;
	char errmsg[200];

	MPI_Init( &argc, &argv );
	Test_Init_No_File();
	
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	/* get buffer space and init table */
	buffer = (int *)malloc( (size * BLOCKSIZE) * (size * BLOCKSIZE) * sizeof(int) );
	table  = (int **)malloc( (size * BLOCKSIZE) * sizeof(int *) );
	if( !buffer || !table ) {
		fprintf( stderr, "Out of memory error!\n" );
		MPI_Abort( MPI_COMM_WORLD, EXIT_FAILURE );
	}
	for( i = 0; i < size * BLOCKSIZE; i++ )
		table[i] = &(buffer[i*size*BLOCKSIZE]);

	/* Determine what rows are my responsibility */
	begin_row = rank * BLOCKSIZE;
	end_row   = (rank + 1) * BLOCKSIZE;
	send_count = recv_count = BLOCKSIZE * size * BLOCKSIZE;

	/* Paint my rows my color */
	for( i = begin_row; i < end_row ; i++ )
		for( j = 0; j < size * BLOCKSIZE; j++ )
			table[i][j] = rank + 10;

	/* Gather everybody's result together - sort of like an */
	/* inefficient allgather */
	for (i = 0; i < size; i++)
		MPI_Gather(table[begin_row], send_count, MPI_INT,
				   table[0], recv_count, MPI_INT, i,
				   MPI_COMM_WORLD);

	/* Everybody should have the same table now. */
	for( i = 0; i < size; i++ )
		for( j = 0; j < BLOCKSIZE; j++ )
			for( k = 0; k < size * BLOCKSIZE; k++ )
				if( table[i*BLOCKSIZE+j][k] != i + 10 ) {
					sprintf(errmsg, "[%d] got %d expected %d for %dth entry in row %d\n",
							rank, table[i*BLOCKSIZE+j][k], i + 10, k, i*BLOCKSIZE + j);
					Test_Message( errmsg );
					Test_Failed( NULL );
				}
	
	Test_Waitforall();
	Test_Global_Summary();

	free( buffer );
	free( table );

	MPI_Finalize();
	exit( EXIT_SUCCESS );
}

/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
