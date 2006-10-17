/*
 * $Id$
 *
 * This file has helper macros for the MPI_Allreduce tests. Moved here so
 * decoupled Allreduce-tests can use it.
 */
#ifndef ALLRED_H
#define ALLRED_H

#define CHECK_VERIFY(MPI_ACTION) do { \
	if (errcnt > 0) {                                                  \
		printf("Found %d errors on %d for MPI_" # MPI_ACTION "\n", \
				errcnt, rank);                             \
		Test_Failed("MPI_" # MPI_ACTION);                          \
	} else                                                             \
		Test_Passed("MPI_" # MPI_ACTION);                          \
	errcnt = 0;                                                        \
} while(0)

#define CHECK_SIMPLE(TYPE, MPI_TYPE, MPI_ACTION, IN, CHECK) do {                   \
	TYPE *in, *out, *sol;                                                      \
	int  i, fnderr = 0;                                                        \
	in = (TYPE *)malloc(count * sizeof(TYPE));                                 \
	out = (TYPE *)malloc(count * sizeof(TYPE));                                \
	sol = (TYPE *)malloc(count * sizeof(TYPE));                                \
	for (i = 0; i < count; i++) {                                              \
		in[i] = IN;                                                        \
		sol[i] = CHECK;                                                    \
		out[i] = 0;                                                        \
	}                                                                          \
	MPI_Allreduce(in, out, count, MPI_ ## MPI_TYPE, MPI_ ## MPI_ACTION, comm); \
	for (i = 0; i < count; i++) {                                              \
		if (out[i] != sol[i]) {                                            \
			errcnt++;                                                  \
			fnderr++;                                                  \
			fprintf(stderr, "(%d) Expected %d got %d\n", world_rank,   \
			        (int)sol[i], (int)out[i]);                         \
		}                                                                  \
	}                                                                          \
	if (fnderr)                                                                \
		fprintf(stderr, "(%d) Error for type MPI_" # MPI_TYPE              \
		        " and op MPI_" # MPI_ACTION "(" # IN ")\n", world_rank);   \
	free(in);                                                                  \
	free(out);                                                                 \
	free(sol);                                                                 \
} while (0)

#define CHECK_SIMPLE_ALL_INT_TESTS(MPI_ACTION, IN, CHECK)                    \
	CHECK_SIMPLE(int, INT, MPI_ACTION, IN, CHECK);                       \
	CHECK_SIMPLE(long, LONG, MPI_ACTION, IN, CHECK);                     \
	CHECK_SIMPLE(short, SHORT, MPI_ACTION, IN, CHECK);                   \
	CHECK_SIMPLE(unsigned short, UNSIGNED_SHORT, MPI_ACTION, IN, CHECK); \
	CHECK_SIMPLE(unsigned, UNSIGNED, MPI_ACTION, IN, CHECK);             \
	CHECK_SIMPLE(unsigned long, UNSIGNED_LONG, MPI_ACTION, IN, CHECK)

#define CHECK_SIMPLE_ALL_INT(MPI_ACTION, IN, CHECK)        \
	CHECK_SIMPLE_ALL_INT_TESTS(MPI_ACTION, IN, CHECK); \
	CHECK_VERIFY(MPI_ACTION)

#define CHECK_SIMPLE_ALL_INT_CHAR(MPI_ACTION, IN, CHECK)          \
	CHECK_SIMPLE_ALL_INT_TESTS(MPI_ACTION, IN, CHECK);        \
	CHECK_SIMPLE(unsigned char, BYTE, MPI_ACTION, IN, CHECK); \
	CHECK_VERIFY(MPI_ACTION)

#define CHECK_SIMPLE_ALL(MPI_ACTION, IN, CHECK)              \
	CHECK_SIMPLE_ALL_INT_TESTS(MPI_ACTION, IN, CHECK);   \
	CHECK_SIMPLE(float, FLOAT, MPI_ACTION, IN, CHECK);   \
	CHECK_SIMPLE(double, DOUBLE, MPI_ACTION, IN, CHECK); \
	CHECK_VERIFY(MPI_ACTION)

#define CHECK_STRUCT(TYPE, MPI_TYPE, MPI_ACTION, IN_A, IN_B, CHECK_A, CHECK_B) do { \
	struct local_test { TYPE a; int b; } *in, *out, *sol;                      \
	int i, fnderr = 0;                                                         \
	in = (struct local_test *)malloc(count * sizeof(struct local_test));       \
	out = (struct local_test *)malloc(count * sizeof(struct local_test));      \
	sol = (struct local_test *)malloc(count * sizeof(struct local_test));      \
	for (i = 0; i < count; i++) {                                              \
		in[i].a = IN_A;                                                    \
		in[i].b = IN_B;                                                    \
		sol[i].a = CHECK_A;                                                \
		sol[i].b = CHECK_B;                                                \
		out[i].a = 0;                                                      \
		out[i].b = -1;                                                     \
	}                                                                          \
	MPI_Allreduce(in, out, count, MPI_ ## MPI_TYPE, MPI_ ## MPI_ACTION, comm); \
	for (i = 0; i < count; i++) {                                              \
		if (out[i].a != sol[i].a || out[i].b != sol[i].b) {                \
			errcnt++;                                                  \
			fnderr++;                                                  \
			fprintf(stderr, "(%d) Expected (%d,%d) got (%d,%d)\n",     \
				world_rank, (int)(sol[i].a), sol[i].b,             \
				(int)(out[i].a), out[i].b);                        \
		}                                                                  \
	}                                                                          \
	if (fnderr)                                                                \
		fprintf( stderr, "(%d) Error for type MPI_" # MPI_TYPE             \
				" and op MPI_" # MPI_ACTION " (%d of %d wrong)\n", \
				world_rank, fnderr, count );                       \
	free(in);                                                                  \
	free(out);                                                                 \
	free(sol);                                                                 \
} while (0)

#define CHECK_STRUCT_ALL(MPI_ACTION, IN_A, IN_B, CHECK_A, CHECK_B)                  \
	CHECK_STRUCT(int, 2INT, MPI_ACTION, IN_A, IN_B, CHECK_A, CHECK_B);          \
	CHECK_STRUCT(long, LONG_INT, MPI_ACTION, IN_A, IN_B, CHECK_A, CHECK_B);     \
	CHECK_STRUCT(short, SHORT_INT, MPI_ACTION, IN_A, IN_B, CHECK_A, CHECK_B);   \
	CHECK_STRUCT(float, FLOAT_INT, MPI_ACTION, IN_A, IN_B, CHECK_A, CHECK_B);   \
	CHECK_STRUCT(double, DOUBLE_INT, MPI_ACTION, IN_A, IN_B, CHECK_A, CHECK_B); \
	CHECK_VERIFY(MPI_ACTION)

#endif
