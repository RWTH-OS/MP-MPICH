/*
 * $Id$
 *
 * Procedures for recording and printing test results
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include "test.h"

static int tests_passed = 0;
static int tests_failed = 0;
static char failed_tests[255][81];
static char suite_name[255];
FILE *fileout = NULL;

void Test_Init(char *suite)
{
	char filename[512];
	int rank;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	sprintf(filename, "%s-%d.out", suite, rank);
	strncpy(suite_name, suite, 255);
	fileout = fopen(filename, "w");
	if (!fileout) {
		fprintf(stderr, "Could not open %s on node %d\n", filename, rank);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
}

void Test_Init_No_File(void)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	fileout = stdout;
	sprintf(suite_name, "none-%i", rank);
}

void Test_Message(char *mess)
{
	fprintf(fileout, "[%s]: %s\n", suite_name, mess);
	fflush(fileout);
}

void Test_Failed(char *test)
{
	if (test != NULL && fileout != NULL) {
		fprintf(fileout, "[%s]: *** Test '%s' Failed! ***\n", suite_name, test);
		strncpy(failed_tests[tests_failed], test, 81);
		fflush(fileout);
	}
	tests_failed++;
}

void Test_Passed(char *test)
{
#ifdef VERBOSE
	fprintf(fileout, "[%s]: Test '%s' Passed.\n", suite_name, test);
	fflush(fileout);
#endif
	tests_passed++;
}

int Summarize_Test_Results(void)
{
	fprintf(fileout, "For test suite '%s':\n", suite_name);
	fprintf(fileout, "Of %d attempted tests, %d passed, %d failed.\n",
			tests_passed + tests_failed, tests_passed, tests_failed);
	if (tests_failed > 0) {
		int i;

		fprintf(fileout, "*** Tests Failed:\n");
		for (i = 0; i < tests_failed; i++)
			fprintf(fileout, "*** %s\n", failed_tests[i]);
	}
	fflush(fileout);

	return tests_failed;
}

int Test_Global_Summary (void)
{
	int all_tests_failed = 0, myrank;

	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Reduce(&tests_failed, &all_tests_failed, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (myrank == 0) {
		if (all_tests_failed > 0) {
			if (strncmp(suite_name, "none-", 5) != 0) {
				printf("Some tests failed, please check %s-*.out.\n", suite_name);
			} else
				printf("Some tests failed.\n");
		} else
			printf(" No Errors\n");
	}
	return (myrank == 0) ? all_tests_failed : tests_failed;
}


void Test_Finalize()
{
	fflush(fileout);
	fclose(fileout);
}

void Print_Filecontent(char *suite, int rank)
{
	char filename[512];
	int ch;
	FILE *fileout = NULL;

	sprintf(filename, "%s-%d.out", suite, rank);
	strncpy(suite_name, suite, 255);
	fileout = fopen(filename, "r");
	do {
		ch = fgetc(fileout);
		if (ch >= 0) /* avoid output of invalid characters */
			putchar(ch);
	} while (feof(fileout) == 0);
	fclose(fileout);
}

/*
 * Wait for every process to pass through this point.  This test is used
 * to make sure that all processes complete, and that a test "passes" because
 * it executed, not because some process failed.
 */
void Test_Waitforall(void)
{
	int m, one, myrank, n;

	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &n);
	one = 1;
	MPI_Allreduce(&one, &m, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	if (m != n) {
		printf("[%d] Expected %d processes to wait at end, got %d\n", myrank,
				n, m);
	}
#ifdef VERBOSE
	if (myrank == 0)
		printf("All processes completed test\n");
#endif
}
