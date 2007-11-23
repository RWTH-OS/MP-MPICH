# Generated automatically from Makefile.in by configure.
ARCH        = LINUX
RANLIB      = ranlib
FILE_SYSTEM = ufs nfs
MAKE        = make
AR          = ar crl
LIBNAME     = /work/rainer/mp-mpich/mp-mpich-15/lib/libmpio.a
NOF77       = 0
NOPROFILE   = 0
MPI         = mpich
PREFIX      = 
FROM_MPICH  = 1
BUILD_MPI_INFO = 
BUILD_MPI_ARRAY = 
DIRS        = mpi-io adio/common
MPIO_DIRS   = mpi-io
ALL_DIRS    = mpi-io mpi-io/fortran mpi2-other/info mpi2-other/info/fortran mpi2-other/array mpi2-other/array/fortran adio/common adio/ad_pfs adio/ad_piofs adio/ad_nfs adio/ad_ufs adio/ad_xfs adio/ad_hfs adio/ad_sfs test
SHELL       = /bin/sh

VPATH=.:$(srcdir)

all: mpio

mpio:
	@$(MAKE) clean
	@$(MAKE) mpiolib
	@$(MAKE) clean
	@if test "$(NOPROFILE)" = 0 ; then\
            $(MAKE) profile; \
        fi
	@$(RANLIB) $(LIBNAME)
	@if test "$(FROM_MPICH)" = 0 ; then \
           echo " "; \
           echo "Completed build of ROMIO. Check the output for errors."; \
           echo " " ;\
           echo "To compile the example programs, cd to the test directory"; \
           echo "and type make."; \
           echo "Run the examples the way you would run any MPI program."; \
           echo "Each program takes the filename as a command-line argument"; \
           echo "'-fname filename'." ; \
           echo " " ;\
        fi

clean:
	@echo "cleaning..." ;\
	dirs=`echo "$(DIRS)"`; \
        for x in $(FILE_SYSTEM) ;\
         do \
             dirs=`echo "$$dirs" adio/ad_$$x` ;\
         done ; \
        if test "$(BUILD_MPI_INFO)" = 1 ; then \
            dirs=`echo "$$dirs" mpi2-other/info` ;\
            if test "$(NOF77)" = 0 ; then\
                dirs=`echo "$$dirs" mpi2-other/info/fortran` ;\
            fi;\
        fi; \
        if test "$(BUILD_MPI_ARRAY)" = 1 ; then \
            dirs=`echo "$$dirs" mpi2-other/array` ;\
            if test "$(NOF77)" = 0 ; then\
                dirs=`echo "$$dirs" mpi2-other/array/fortran` ;\
            fi;\
        fi; \
        if test "$(NOF77)" = 0 ; then\
            dirs=`echo "$$dirs" mpi-io/fortran` ;\
        fi;\
        for dir in $$dirs ;\
         do \
            echo "directory $$dir" ;\
            (cd $$dir; rm -f *.o) ;\
         done; \
         echo "directory test" ;\
         (cd test; $(MAKE) clean);\

mpiolib:
	@if test "$(FROM_MPICH)" = 0 ; then \
            echo " " ;\
            echo "deleting previously built library if it exists" ;\
            rm -f $(LIBNAME) ;\
        fi; \
        dirs=`echo "$(DIRS)"`; \
        for x in $(FILE_SYSTEM) ;\
         do \
             dirs=`echo "$$dirs" adio/ad_$$x` ;\
         done ; \
        if test "$(BUILD_MPI_INFO)" = 1 ; then \
            dirs=`echo "$$dirs" mpi2-other/info` ;\
            if test "$(NOF77)" = 0 ; then \
                dirs=`echo "$$dirs" mpi2-other/info/fortran` ;\
            fi; \
        fi; \
        if test "$(BUILD_MPI_ARRAY)" = 1 ; then \
            dirs=`echo "$$dirs" mpi2-other/array` ;\
            if test "$(NOF77)" = 0 ; then \
                dirs=`echo "$$dirs" mpi2-other/array/fortran` ;\
            fi; \
        fi; \
        if test "$(NOF77)" = 0 ; then \
            dirs=`echo "$$dirs" mpi-io/fortran` ;\
        fi; \
        for dir in $$dirs ;\
         do \
            echo " " ;\
            echo compiling ROMIO in directory $$dir ;\
            (cd $$dir; $(MAKE) -j 5 ;) ;\
            if [ $$? != 0 ] ; then \
                echo "Make failed in directory $$dir" ; exit 1 ;\
            fi ;\
         done

# When we need to build the profiling library separately (due 
# to missing weak symbols), we need to do so for the ad_xy libraries,
# too, as our MPE logging library performs a logging on the ADIO level.
profile: 
	@if [ "1" = 0 ] ; then \
	   echo "";\
	   echo "*#  no weak symbols available - building profiling library";\
           dirs=`echo "$(MPIO_DIRS)"`; \
           if test "$(BUILD_MPI_INFO)" = 1 ; then \
               dirs=`echo "$$dirs" mpi2-other/info` ;\
               if test "$(NOF77)" = 0 ; then\
                   dirs=`echo "$$dirs" mpi2-other/info/fortran` ;\
               fi;\
           fi; \
           if test "$(BUILD_MPI_ARRAY)" = 1 ; then \
               dirs=`echo "$$dirs" mpi2-other/array` ;\
               if test "$(NOF77)" = 0 ; then\
                   dirs=`echo "$$dirs" mpi2-other/array/fortran` ;\
               fi;\
           fi; \
           if test "$(NOF77)" = 0 ; then\
               dirs=`echo "$$dirs" mpi-io/fortran` ;\
           fi;\
	   for x in $(FILE_SYSTEM) ;\
           do \
             dirs=`echo "$$dirs" adio/ad_$$x` ;\
           done ; \
           for dir in $$dirs ;\
            do \
               echo " " ;\
               echo building profiling interface in directory $$dir ;\
               (cd $$dir; \
                $(MAKE) profile MPIOPROFILE="-DMPIO_BUILD_PROFILING";) ;\
                if [ $$? != 0 ] ; then \
                    echo "Make failed in directory $$dir" ; exit 1 ;\
                fi ;\
            done ;\
	    $(MAKE) clean; \
	else \
	  echo "" ; \
	  echo "*#  weak symbols available - profiling via main MPIO library" ; \
	  ( cd /work/rainer/mp-mpich/mp-mpich-15/lib ;\
	  if [ -n "libad_ufs.a" -a -r "libad_ufs.a" -a ! -r "libpad_ufs.a" ] ; then \
		ln -s libad_ufs.a libpad_ufs.a ;\
	  fi ;\
	  if [ -n "libad_nfs.a" -a -r "libad_nfs.a" -a ! -r "libpad_nfs.a" ] ; then \
		ln -s libad_nfs.a libpad_nfs.a ;\
	  fi ;\
	  if [ -n "libad_ufs.a" -a -r "libad_ufs.a" -a ! -r "libpad_ufs.a" ] ; then \
		ln -s   ;\
	  fi ) ;\
	fi

distclean:
	@(dirs=`echo "$(ALL_DIRS)"`; \
	rm -f config.status Makefile *~ *.log romio.tar* ;\
	for dir in $$dirs ;\
         do \
            echo "     cleaning directory $$dir" ;\
            (cd $$dir; rm -f *.o Makefile *~) ;\
         done;\
	(cd test; rm -f .P* PI* misc.c large_file.c fcoll_test.f fmisc.f fperf.f \
	mpif.h simple async coll_test coll_perf misc perf fcoll_test fperf fmisc \
	file_info excl large_array large_file noncontig i_noncontig noncontig_coll \
	atomicity *.out runtests ) ;\
        echo "     cleaning directory include" ;\
        ( cd include; rm -f *.h );\
        echo "     cleaning directory util" ;\
	( cd util; rm -f romioinstall *~) )

install:
	$(MAKE) clean
	@./util/romioinstall -prefix=$(PREFIX)

romio_lflags:
	@echo ''

romio_liblist:
	@echo ' -lad_ufs -lad_nfs -ladio '

romio_devlibs:
	@echo ' -lad_ufs -lad_nfs -ladio'

romio_devplibs:
	@echo ' -lpad_ufs -lpad_nfs'

romio_libpath:
	@echo ''

romio_tcflags:
	@echo ''

romio_tcppflags:
	@echo ''

romio_tfflags:
	@echo ''
