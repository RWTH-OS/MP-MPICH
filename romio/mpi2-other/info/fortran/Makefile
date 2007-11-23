# Generated automatically from Makefile.in by configure.
CC          = /work/rainer/mp-mpich/mp-mpich-15/bin/mpicc
AR          = ar crl
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libmpio.a
RANLIB      = ranlib
srcdir      = /work/rainer/mp-mpich/mp-mpich-15/romio/mpi2-other/info/fortran

INCLUDE_DIR = -I/work/rainer/mp-mpich/mp-mpich-15/include -I../../../include -I${srcdir}/../../../adio/include -I${srcdir}/../../../mpi-io
CFLAGS      =   -fPIC -DLINUX -O -DNO_AIO -DFORTRANDOUBLEUNDERSCORE -DNFS -DUFS -DNEEDS_MPI_TEST -DMPICH  -DHAVE_LONG_LONG_64=1 -DHAVE_MPI_LONG_LONG_INT=1 -DHAVE_MPI_INFO=1 -DHAVE_MPI_DARRAY_SUBARRAY=1 -DHAVE_WEAK_SYMBOLS=1 -DHAVE_PRAGMA_WEAK=1 $(MPIOPROFILE) $(INCLUDE_DIR)

VPATH=.:$(srcdir)

MPIO_OBJECTS = info_createf.o info_dupf.o info_getf.o info_getnthf.o \
      info_setf.o info_deletef.o info_freef.o info_getnksf.o info_getvlnf.o

MPIO_TMP_POBJECTS = info_createf.p info_dupf.p info_getf.p info_getnthf.p \
      info_setf.p info_deletef.p info_freef.p info_getnksf.p info_getvlnf.p

MPIO_REAL_POBJECTS = _info_createf.o _info_dupf.o _info_getf.o _info_getnthf.o\
    _info_setf.o _info_deletef.o _info_freef.o _info_getnksf.o _info_getvlnf.o

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
