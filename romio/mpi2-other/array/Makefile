# Generated automatically from Makefile.in by configure.
CC          = /work/rainer/mp-mpich/mp-mpich-15/bin/mpicc
AR          = ar crl
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libmpio.a
RANLIB      = ranlib
srcdir      = /work/rainer/mp-mpich/mp-mpich-15/romio/mpi2-other/array

INCLUDE_DIR = -I/work/rainer/mp-mpich/mp-mpich-15/include -I../../include -I${srcdir}/../../mpi-io -I${srcdir}/../../adio/include
CFLAGS      =   -fPIC -DLINUX -O -DNO_AIO -DFORTRANDOUBLEUNDERSCORE -DNFS -DUFS -DNEEDS_MPI_TEST -DMPICH  -DHAVE_LONG_LONG_64=1 -DHAVE_MPI_LONG_LONG_INT=1 -DHAVE_MPI_INFO=1 -DHAVE_MPI_DARRAY_SUBARRAY=1 -DHAVE_WEAK_SYMBOLS=1 -DHAVE_PRAGMA_WEAK=1 $(MPIOPROFILE) $(INCLUDE_DIR)

VPATH=.:$(srcdir)

MPIO_OBJECTS = subarray.o darray.o

MPIO_TMP_POBJECTS = subarray.p darray.p

MPIO_REAL_POBJECTS = _subarray.o _darray.o

all: $(LIBNAME)

.SUFFIXES: $(SUFFIXES) .p

.c.o:
	$(CC) $(CFLAGS) -c $<

.c.p:
	@cp $(srcdir)/$*.c _$*.c
	$(CC) $(CFLAGS) -c _$*.c
	@rm -f _$*.c

profile: $(MPIO_TMP_POBJECTS)
	$(AR) $(LIBNAME) $(MPIO_REAL_POBJECTS)
	@rm -f _*.o

$(LIBNAME): $(MPIO_OBJECTS)
	$(AR) $(LIBNAME) $(MPIO_OBJECTS)

clean:
	@rm -f *.o
