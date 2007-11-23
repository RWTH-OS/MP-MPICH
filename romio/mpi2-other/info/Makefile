# Generated automatically from Makefile.in by configure.
CC          = /work/rainer/mp-mpich/mp-mpich-15/bin/mpicc
AR          = ar crl
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libmpio.a
RANLIB      = ranlib
srcdir      = /work/rainer/mp-mpich/mp-mpich-15/romio/mpi2-other/info

INCLUDE_DIR = -I/work/rainer/mp-mpich/mp-mpich-15/include -I../../include -I${srcdir}/../../mpi-io -I${srcdir}/../../adio/include
CFLAGS      = -D__MPI_INFO_SRC   -fPIC -DLINUX -O -DNO_AIO -DFORTRANDOUBLEUNDERSCORE -DNFS -DUFS -DNEEDS_MPI_TEST -DMPICH  -DHAVE_LONG_LONG_64=1 -DHAVE_MPI_LONG_LONG_INT=1 -DHAVE_MPI_INFO=1 -DHAVE_MPI_DARRAY_SUBARRAY=1 -DHAVE_WEAK_SYMBOLS=1 -DHAVE_PRAGMA_WEAK=1 $(MPIOPROFILE) $(INCLUDE_DIR)

VPATH=.:$(srcdir)

MPIO_OBJECTS = info_create.o info_dup.o info_get.o info_getnth.o \
      info_set.o info_delete.o info_free.o info_getnks.o info_getvln.o \
      info_c2f.o info_f2c.o

MPIO_TMP_POBJECTS = info_create.p info_dup.p info_get.p info_getnth.p \
      info_set.p info_delete.p info_free.p info_getnks.p info_getvln.p \
      info_c2f.p info_f2c.p

MPIO_REAL_POBJECTS = _info_create.o _info_dup.o _info_get.o _info_getnth.o \
      _info_set.o _info_delete.o _info_free.o _info_getnks.o _info_getvln.o \
      _info_c2f.o _info_f2c.o

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
