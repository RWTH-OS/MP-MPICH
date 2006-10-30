/*
 * $Id$
 *
 * Correctness test for MPI_Reduce operations. Iterates over all available
 * basic datatypes and default reduce operations, vector size can be adjusted
 * (MAXSIZE).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mpi.h"
#include "../util/test.h"

#define MAXSIZE 4096

/* Define for more verbose output */
#undef VERBOSE
/* Define for VERY verbose error reporting (with dumps of differences in the
 * data) */
#undef INSANE_ERROR_REPORTING
/* Define for simple pseudo-random numbers */
#undef SIMPLE_RANDOM

#ifdef SIMPLE_RANDOM
#define rand myrand
#define srand mysrand
#define SEED 3
#else
#define SEED time(NULL)
#endif

#define NUMOPS (sizeof(Ops) / sizeof(MPI_Op))
#define NUMTYPES (sizeof(Types) / sizeof(int))

const MPI_Op Ops[] = { MPI_MAX, MPI_MIN, MPI_SUM, MPI_PROD, MPI_BAND,
	MPI_BOR, MPI_BXOR, MPI_LAND, MPI_LOR, MPI_LXOR
};

const MPI_Datatype Types[] = { MPI_BYTE, MPI_INT, MPI_SHORT, MPI_LONG,
	MPI_UNSIGNED, MPI_UNSIGNED_SHORT, MPI_UNSIGNED_LONG,
#ifdef TEST_FLOAT
	MPI_FLOAT, MPI_DOUBLE
#endif
};

const int Sizes[] = { sizeof(char), sizeof(int), sizeof(short),
	sizeof(long), sizeof(unsigned int), sizeof(unsigned short),
	sizeof(unsigned long), sizeof(float), sizeof(double)
};

char *OpNames[] = { "MPI_MAX", "MPI_MIN", "MPI_SUM", "MPI_PROD",
	"MPI_BAND", "MPI_BOR", "MPI_BXOR", "MPI_LAND", "MPI_LOR", "MPI_LXOR"
};

char *TypeNames[] = { "MPI_BYTE", "MPI_INT", "MPI_SHORT", "MPI_LONG",
	"MPI_UNSIGNED", "MPI_UNSIGNED_SHORT", "MPI_UNSIGNED_LONG", "MPI_FLOAT",
	"MPI_DOUBLE"
};

int OpValid[][9] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1},	/* MAX */
	{1, 1, 1, 1, 1, 1, 1, 1, 1},	/* MIN */
	{1, 1, 1, 1, 1, 1, 1, 1, 1},	/* SUM */
	{1, 1, 1, 1, 1, 1, 1, 1, 1},	/* PROD */
	{1, 1, 1, 1, 1, 1, 1, 0, 0},	/* BAND */
	{1, 1, 1, 1, 1, 1, 1, 0, 0},	/* BOR */
	{1, 1, 1, 1, 1, 1, 1, 0, 0},	/* BXOR */
	{1, 1, 1, 1, 1, 1, 1, 0, 0},	/* LAND */
	{1, 1, 1, 1, 1, 1, 1, 0, 0},	/* LOR */
	{1, 1, 1, 1, 1, 1, 1, 0, 0}	/* LXOR */
};

MPI_Op *default_ops;

typedef struct {
	void *inbuf, *out1, *out2;
} buftype;

buftype Buffers[NUMTYPES];

int NumProcs, MyRank;

extern void MPICH_DEFAULT_MINF(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_MAXF(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_SUM(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_PROD(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_LAND(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_BAND(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_LOR(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_BOR(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_LXOR(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);
extern void MPICH_DEFAULT_BXOR(void *invec, void *inoutvec, int *Len,
		MPI_Datatype *type);

#ifdef SIMPLE_RANDOM
static unsigned long next = 1;

/* RAND_MAX assumed to be 32767 */
int myrand(void) {
	next = next * 1103515245 + 12345;
	return((unsigned)(next/65536) % 32768);
}

void mysrand(unsigned seed) {
	next = seed;
}
#endif

#ifdef INSANE_ERROR_REPORTING
void diff_array(void *one, void *other, int len, MPI_Datatype type)
{
	int i, c = 0;

	switch (type) {
	case MPI_LOGICAL:{
			MPI_Fint *a = (MPI_Fint *) one;
			MPI_Fint *b = (MPI_Fint *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_INT:{
			int *a = (int *) one;
			int *b = (int *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_UNSIGNED:{
			unsigned *a = (unsigned *) one;
			unsigned *b = (unsigned *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_LONG:{
			long *a = (long *) one;
			long *b = (long *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %ld != %ld", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPI_LONG_LONG_INT:{
			long long *a = (long long *) one;
			long long *b = (long long *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %lld != %lld", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
#endif
	case MPI_UNSIGNED_LONG:{
			unsigned long *a = (unsigned long *) one;
			unsigned long *b = (unsigned long *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %ld != %ld", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_SHORT:{
			short *a = (short *) one;
			short *b = (short *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_UNSIGNED_SHORT:{
			unsigned short *a = (unsigned short *) one;
			unsigned short *b = (unsigned short *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_CHAR:{
			char *a = (char *) one;
			char *b = (char *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_UNSIGNED_CHAR:{
			unsigned char *a = (unsigned char *) one;
			unsigned char *b = (unsigned char *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	case MPI_BYTE:{
			unsigned char *a = (unsigned char *) one;
			unsigned char *b = (unsigned char *) other;
			for (i = 0; i < len; i++)
				if (a[i] != b[i])
					printf("%s%i: %d != %d", (c++)?", ":"", i, a[i], b[i]);
			break;
		}
	default:
		printf("Not supported data type. Can't output...");
		break;
	}
	printf("\n");
}
#endif

void AllocBuffers_aligned(void)
{
	MPI_Info info;
	char value[16];
	unsigned int i;

	MPI_Info_create(&info);
	sprintf(value, "%d", 32);
	MPI_Info_set(info, "alignment", value);

	for (i = 0; i < NUMTYPES; i++) {
		MPI_Alloc_mem(MAXSIZE * Sizes[i], info, &Buffers[i].inbuf);
		MPI_Alloc_mem(MAXSIZE * Sizes[i], info, &Buffers[i].out1);
		MPI_Alloc_mem(MAXSIZE * Sizes[i], info, &Buffers[i].out2);
	}
}

void AllocBuffers(void)
{
	unsigned int i;

	for (i = 0; i < NUMTYPES; i++) {
		Buffers[i].inbuf = malloc(MAXSIZE * Sizes[i]);
		Buffers[i].out1 = malloc(MAXSIZE * Sizes[i]);
		Buffers[i].out2 = malloc(MAXSIZE * Sizes[i]);
	}
}

/* Initialize buffers for numerical reduce operations. */
void InitBuffers_num(void)
{
	unsigned int j;

	for (j = 0; j < MAXSIZE; ++j) {
		/* signed */
		((char *) Buffers[0].inbuf)[j] = (char) rand();

		((int *) Buffers[1].inbuf)[j] = (int) (rand() - RAND_MAX / 2);

		((short *) Buffers[2].inbuf)[j] =
				(short) (rand() - RAND_MAX / 2);

		((long *) Buffers[3].inbuf)[j] = (long) (rand() - RAND_MAX / 2);

		/* unsigned */
		((int *) Buffers[4].inbuf)[j] = (int) rand();

		((short *) Buffers[5].inbuf)[j] = (short) rand();

		((long *) Buffers[6].inbuf)[j] = (long) rand();

#ifdef TEST_FLOAT
		/* float */
		((float *) Buffers[7].inbuf)[j] =
				(float) rand() - (float) RAND_MAX / (float) 2.0;

		((double *) Buffers[8].inbuf)[j] =
				(double) rand() - (double) RAND_MAX / 2.0;
#endif
	}

}

/* Initialize buffers for logical reduce operations. */
void InitBuffers_log(void)
{
	int j;

	for (j = 0; j < MAXSIZE; ++j) {
		/* signed */
		((char *) Buffers[0].inbuf)[j] = (char) (rand() & 1);

		((int *) Buffers[1].inbuf)[j] = (int) (rand() & 1);

		((short *) Buffers[2].inbuf)[j] = (short) (rand() & 1);

		((long *) Buffers[3].inbuf)[j] = (long) (rand() & 1);

		/* unsigned */
		((int *) Buffers[4].inbuf)[j] = (int) (rand() & 1);

		((short *) Buffers[5].inbuf)[j] = (short) (rand() & 1);

		((long *) Buffers[6].inbuf)[j] = (long) (rand() & 1);
	}

}

void test_operator_with_datatype(int op, int type)
{
	int res1, res2, len;

	if (!OpValid[op][type])
		return;

#ifdef VERBOSE
	if (MyRank == 0) {
		printf("# Op \"%s\", Type \"%s\"\n", OpNames[op],
				TypeNames[type]);
		fflush(stdout);
	}
#endif

	for (len = 1; len <= MAXSIZE; len *= 2) {
		/* test optimized MP-MPICH implementation */
		res1 = MPI_Reduce(Buffers[type].inbuf, Buffers[type].out1, len,
				Types[type], Ops[op], 0, MPI_COMM_WORLD);

		/* test default MPICH implementation */
		res2 = MPI_Reduce(Buffers[type].inbuf, Buffers[type].out2, len,
				Types[type], default_ops[op], 0,
				MPI_COMM_WORLD);

		if (res1 != MPI_SUCCESS)
			printf("WARNING: Op %s, Type %s failed with MP-MPICH optimized operations.\n",
					OpNames[op], TypeNames[type]);
		else if (res2 != MPI_SUCCESS)
			printf("WARNING: Op %s, Type %s failed with default MPICH operations.\n",
					OpNames[op], TypeNames[type]);
		else if (MyRank == 0 && memcmp(Buffers[type].out1,
				Buffers[type].out2, len * Sizes[type])) {
			printf("ERROR: Op %s, Type %s, Size %d: The buffers differ!\n",
					OpNames[op], TypeNames[type], len);
#ifdef INSANE_ERROR_REPORTING
			diff_array(Buffers[type].out1, Buffers[type].out2, len, Types[type]);
#endif
			Test_Failed(NULL);
		}
	}
}

int main(int argc, char **argv)
{
	unsigned int type, op;

	MPI_Init(&argc, &argv);
	Test_Init_No_File();
	MPI_Comm_size(MPI_COMM_WORLD, &NumProcs);
	MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);

	MPI_Errhandler_set(MPI_COMM_WORLD, MPIR_ERRORS_WARN);

	default_ops = malloc(NUMOPS * sizeof(MPI_Op));
	MPI_Op_create(MPICH_DEFAULT_MAXF, 1, &default_ops[0]);
	MPI_Op_create(MPICH_DEFAULT_MINF, 1, &default_ops[1]);
	MPI_Op_create(MPICH_DEFAULT_SUM, 1, &default_ops[2]);
	MPI_Op_create(MPICH_DEFAULT_PROD, 1, &default_ops[3]);
	MPI_Op_create(MPICH_DEFAULT_BAND, 1, &default_ops[4]);
	MPI_Op_create(MPICH_DEFAULT_BOR, 1, &default_ops[5]);
	MPI_Op_create(MPICH_DEFAULT_BXOR, 1, &default_ops[6]);
	MPI_Op_create(MPICH_DEFAULT_LAND, 1, &default_ops[7]);
	MPI_Op_create(MPICH_DEFAULT_LOR, 1, &default_ops[8]);
	MPI_Op_create(MPICH_DEFAULT_LXOR, 1, &default_ops[9]);

	srand(MyRank * SEED);
	AllocBuffers_aligned();

	/* loop over operation type */
	for (op = 0; op < NUMOPS; ++op) {
#ifdef VERBOSE
		if (MyRank == 0)
			printf("# Testing reduce operation \"%s\"\n",
					OpNames[op]);
#endif

		if (op == 7)
			InitBuffers_log();
		else if (op == 0)
			InitBuffers_num();

		/* loop over data type */
		for (type = 0; type < NUMTYPES; ++type)
			test_operator_with_datatype(op, type);
	}

	Test_Global_Summary();
	MPI_Finalize();
	free(default_ops);
	return 0;
}
