# Generated automatically from Makefile.in by configure.
CC          = /work/rainer/mp-mpich/mp-mpich-15/bin/mpicc
AR          = ar crl
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libad_nfs.a
PLIBNAME    = /work/rainer/mp-mpich/mp-mpich-15/lib/libpad_nfs.a
srcdir      = /work/rainer/mp-mpich/mp-mpich-15/romio/adio/ad_nfs

INCLUDE_DIR = -I/work/rainer/mp-mpich/mp-mpich-15/include -I${srcdir}/../include
CFLAGS      =   -fPIC -DLINUX -O -DNO_AIO -DFORTRANDOUBLEUNDERSCORE -DNFS -DUFS -DNEEDS_MPI_TEST -DMPICH  -DHAVE_LONG_LONG_64=1 -DHAVE_MPI_LONG_LONG_INT=1 -DHAVE_MPI_INFO=1 -DHAVE_MPI_DARRAY_SUBARRAY=1 -DHAVE_WEAK_SYMBOLS=1 -DHAVE_PRAGMA_WEAK=1 $(INCLUDE_DIR) $(MPIOPROFILE) 

VPATH=.:$(srcdir)

AD_NFS_OBJECTS = ad_nfs_close.o ad_nfs_read.o ad_nfs_rdcoll.o \
      ad_nfs_wrcoll.o ad_nfs_open.o ad_nfs_write.o ad_nfs_done.o \
      ad_nfs_fcntl.o ad_nfs_iread.o ad_nfs_iwrite.o ad_nfs_wait.o \
      ad_nfs_flush.o ad_nfs_seek.o ad_nfs_resize.o ad_nfs_hints.o \
      ad_nfs_setsh.o ad_nfs_getsh.o 

default: $(LIBNAME)

profile: $(PLIBNAME)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(LIBNAME): $(AD_NFS_OBJECTS)
	$(AR) $(LIBNAME) $(AD_NFS_OBJECTS)

$(PLIBNAME): $(AD_NFS_OBJECTS)
	$(AR) $(PLIBNAME) $(AD_NFS_OBJECTS)

clean:
	@rm -f *.o
