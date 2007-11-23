# Generated automatically from Makefile.in by configure.
CC          = /work/rainer/mp-mpich/mp-mpich-15/bin/mpicc
AR          = ar crl
RANLIB      = ranlib
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libmpio.a
srcdir      = /work/rainer/mp-mpich/mp-mpich-15/romio/mpi2-other/array/fortran

INCLUDE_DIR = -I/work/rainer/mp-mpich/mp-mpich-15/include -I../../../include -I${srcdir}/../../../mpi-io -I${srcdir}/../../../adio/include
CFLAGS      =   -fPIC -DLINUX -O -DNO_AIO -DFORTRANDOUBLEUNDERSCORE -DNFS -DUFS -DNEEDS_MPI_TEST -DMPICH  -DHAVE_LONG_LONG_64=1 -DHAVE_MPI_LONG_LONG_INT=1 -DHAVE_MPI_INFO=1 -DHAVE_MPI_DARRAY_SUBARRAY=1 -DHAVE_WEAK_SYMBOLS=1 -DHAVE_PRAGMA_WEAK=1 $(MPIOPROFILE) $(INCLUDE_DIR)

VPATH=.:$(srcdir)

MPIO_FOBJECTS = subarrayf.o darrayf.o

MPIO_TMP_PFOBJECTS = subarrayf.p darrayf.p

MPIO_REAL_PFOBJECTS = _subarrayf.o _darrayf.o

all: $(LIBNAME)

.SUFFIXES: $(SUFFIXES) .p

.c.o:
	$(CC) $(CFLAGS) -c $<

.c.p:
	@cp $(srcdir)/$*.c _$*.c
	$(CC) $(CFLAGS) -c _$*.c
	@rm -f _$*.c

profile: $(MPIO_TMP_PFOBJECTS)
	$(AR) $(LIBNAME) $(MPIO_REAL_PFOBJECTS)
	@rm -f _*.o

$(LIBNAME): $(MPIO_FOBJECTS)
	$(AR) $(LIBNAME) $(MPIO_FOBJECTS)

clean: 
	@rm -f *.o
