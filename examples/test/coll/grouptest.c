/*
 * $Id$
 */

#include <stdio.h>
#include "mpi.h"
#include "../util/test.h"

#define GROUPS 3

int main(int argc, char **argv)
{
	int rank, size, i;
	MPI_Group groupall, groupunion, newgroup, group[GROUPS];
	MPI_Comm newcomm;
	int ranks[GROUPS][100];
	int nranks[GROUPS] = { 0, 0, 0 };

	MPI_Init(&argc, &argv);
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_group(MPI_COMM_WORLD, &groupall);

	/* Divide groups */
	for (i = 0; i < size; i++)
		ranks[i % GROUPS][nranks[i % GROUPS]++] = i;

	for (i = 0; i < GROUPS; i++)
		MPI_Group_incl(groupall, nranks[i], ranks[i], &group[i]);

	MPI_Group_difference(groupall, group[1], &groupunion);

	MPI_Comm_create(MPI_COMM_WORLD, group[2], &newcomm);
	newgroup = MPI_GROUP_NULL;
	if (newcomm != MPI_COMM_NULL)
	{
		/* If we don't belong to group[2], this would fail */
		MPI_Comm_group(newcomm, &newgroup);
	}

	/* Free the groups */
	MPI_Group_free(&groupall);
	for (i = 0; i < GROUPS; i++)
		MPI_Group_free(&group[i]);
	MPI_Group_free(&groupunion);
	if (newgroup != MPI_GROUP_NULL)
	{
		MPI_Group_free(&newgroup);
	}

	/* Free the communicator */
	if (newcomm != MPI_COMM_NULL)
		MPI_Comm_free(&newcomm);
	Test_Waitforall();
	Test_Global_Summary();
	MPI_Finalize();
	return 0;
}
