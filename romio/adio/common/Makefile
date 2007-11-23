# Generated automatically from Makefile.in by configure.
CC          = /work/rainer/mp-mpich/mp-mpich-15/bin/mpicc
AR          = ar crl
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libadio.a
MPI         = mpich
srcdir      = /work/rainer/mp-mpich/mp-mpich-15/romio/adio/common

INCLUDE_DIR = -I/work/rainer/mp-mpich/mp-mpich-15/include -I${srcdir}/../include
CFLAGS      =   -fPIC -DLINUX -O -DNO_AIO -DFORTRANDOUBLEUNDERSCORE -DNFS -DUFS -DNEEDS_MPI_TEST -DMPICH  -DHAVE_LONG_LONG_64=1 -DHAVE_MPI_LONG_LONG_INT=1 -DHAVE_MPI_INFO=1 -DHAVE_MPI_DARRAY_SUBARRAY=1 -DHAVE_WEAK_SYMBOLS=1 -DHAVE_PRAGMA_WEAK=1 $(INCLUDE_DIR)

VPATH=.:$(srcdir)

AD_OBJECTS = ad_close.o ad_init.o ad_end.o ad_open.o setfn.o flatten.o \
      async_list.o req_malloc.o malloc.o ad_read_coll.o eof_offset.o \
      ad_write_coll.o lock.o ad_read_str.o ad_write_str.o iscontig.o \
      get_fp_posn.o ad_seek.o ad_delete.o ad_flush.o ad_hints.o \
      ad_fstype.o ad_get_sh_fp.o ad_set_sh_fp.o shfp_fname.o byte_offset.o

all: $(LIBNAME)

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $<

$(LIBNAME): $(AD_OBJECTS)
	$(AR) $(LIBNAME) $(AD_OBJECTS)
