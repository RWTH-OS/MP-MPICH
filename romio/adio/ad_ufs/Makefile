# Generated automatically from Makefile.in by configure.
CC          = /work/rainer/mp-mpich/mp-mpich-15/bin/mpicc
AR          = ar crl
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libad_ufs.a
PLIBNAME    = /work/rainer/mp-mpich/mp-mpich-15/lib/libpad_ufs.a
srcdir      = /work/rainer/mp-mpich/mp-mpich-15/romio/adio/ad_ufs

INCLUDE_DIR = -I/work/rainer/mp-mpich/mp-mpich-15/include -I${srcdir}/../include
CFLAGS      =   -fPIC -DLINUX -O -DNO_AIO -DFORTRANDOUBLEUNDERSCORE -DNFS -DUFS -DNEEDS_MPI_TEST -DMPICH  -DHAVE_LONG_LONG_64=1 -DHAVE_MPI_LONG_LONG_INT=1 -DHAVE_MPI_INFO=1 -DHAVE_MPI_DARRAY_SUBARRAY=1 -DHAVE_WEAK_SYMBOLS=1 -DHAVE_PRAGMA_WEAK=1 $(INCLUDE_DIR) $(MPIOPROFILE) 

VPATH=.:$(srcdir)

AD_UFS_OBJECTS = ad_ufs_close.o ad_ufs_read.o ad_ufs_rdcoll.o \
      ad_ufs_wrcoll.o ad_ufs_open.o ad_ufs_write.o ad_ufs_done.o \
      ad_ufs_fcntl.o ad_ufs_iread.o ad_ufs_iwrite.o ad_ufs_wait.o \
      ad_ufs_flush.o ad_ufs_seek.o ad_ufs_resize.o ad_ufs_hints.o

default: $(LIBNAME)

profile: $(PLIBNAME)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(LIBNAME): $(AD_UFS_OBJECTS)
	$(AR) $(LIBNAME) $(AD_UFS_OBJECTS)

$(PLIBNAME): $(AD_UFS_OBJECTS)
	$(AR) $(PLIBNAME) $(AD_UFS_OBJECTS)

clean:
	@rm -f *.o
