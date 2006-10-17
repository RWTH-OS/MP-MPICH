dnl $Id$
dnl
dnl Additional macros for using autoconf to build configure scripts
dnl
dnl NOTES ON ADDING TO THIS
dnl It is important to end ALL definitions with "dnl" to insure that 
dnl there are NO blank lines before the "/bin/sh" in the configure script.
dnl
dnl Some systems (particularly parallel systems) do not return correct
dnl values for exit codes; for this reason, it is better to get the
dnl sizes by running the programs and then comparing them
dnl
dnl PAC_GET_TYPE_SIZE(typename,var_for_size)
dnl
dnl sets var_for_size to the size.  Ignores if the size cannot be determined
dnl Also sets typename_len to the size; if that is already set, just uses
dnl that
dnl
AC_DEFUN([PAC_GET_TYPE_SIZE],
[Pac_name="$1"
 Pac_varname=`echo "$Pac_name" | sed -e 's/ /_/g' -e 's/\*/star/g'`
eval Pac_testval=\$"${Pac_varname}_len"
if test -n "$Pac_testval" ; then
    Pac_CV_NAME=$Pac_testval
else
 AC_MSG_CHECKING([for size of $Pac_name])
 /bin/rm -f conftestval
 AC_RUN_IFELSE(AC_LANG_PROGRAM([[#include <stdio.h>]],
[[
  FILE *f=fopen("conftestval","w");
  if (!f) exit(1);
  fprintf( f, "%d\n", sizeof($Pac_name));
]]),Pac_CV_NAME=`cat conftestval`,Pac_CV_NAME="")
 if test -n "$Pac_CV_NAME" -a "$Pac_CV_NAME" != 0 ; then
    AC_MSG_RESULT($Pac_CV_NAME)
    eval ${Pac_varname}_len=$Pac_CV_NAME
 else
    AC_MSG_RESULT(unavailable)
 fi
fi
$2=$Pac_CV_NAME
])dnl
dnl
dnl
dnl Define test for 64-bit pointers
dnl
AC_DEFUN([PAC_POINTER_64_BITS],
[
pointersize=""
PAC_GET_TYPE_SIZE(void *,pointersize)
AC_MSG_CHECKING([for pointers greater than 32 bits])
if test -z "$pointersize" ; then
    AC_MSG_RESULT(can not determine; assuming not)
elif test $pointersize -gt 4 ; then
    AC_DEFINE(POINTER_64_BITS,1,[Define if void * is 8 bytes])
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
])dnl
dnl
AC_DEFUN([PAC_INT_LT_POINTER],[
intsize=""
PAC_GET_TYPE_SIZE(int,intsize)
if test -z "$pointersize" ; then
    PAC_GET_TYPE_SIZE(void *,pointersize)
fi
AC_MSG_CHECKING([for int large enough for pointers])
if test -n "$pointersize" -a -n "$intsize" ; then
    if test $pointersize -le $intsize ; then
       AC_MSG_RESULT(yes)
    else
       AC_DEFINE(INT_LT_POINTER,1,[Define if an int is smaller than void *])
       AC_MSG_RESULT(no)
    fi
else
    AC_MSG_RESULT(can not determine; assuming it is)
fi
])dnl
dnl
dnl Define the test for the long long type
dnl This is made more interesting because some compilers implement it, 
dnl but not correctly.  If they can't do it right, turn it off.
AC_DEFUN([PAC_LONG_LONG_INT],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_MSG_CHECKING([for long long])
AC_RUN_IFELSE(AC_LANG_PROGRAM(,[[
/* See long double test; this handles the possibility that long long
   has the same problem on some systems */
exit(sizeof(long long) < sizeof(long)); }]]),
	AC_MSG_RESULT(yes)
	has_long_long=1,
	AC_MSG_RESULT(no)
	has_long_long=0,
	AC_MSG_RESULT(yes)
	has_long_long=1)
if test "$has_long_long" = 1 ; then
   AC_MSG_CHECKING(that compiler can handle loops with long long)
   AC_RUN_IFELSE(AC_LANG_PROGRAM([[
void MPIR_SUM_ext( invec, inoutvec, len )
void *invec, *inoutvec;
int  len;
{
    int i;
    long long *a = (long long *)inoutvec; long long *b = (long long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = a[i]+b[i];
}]],),
	AC_MSG_RESULT(yes),
        AC_MSG_RESULT(no!)
        has_long_long=0,
	AC_MSG_RESULT(yes)
   )
fi
if test "$has_long_long" = 1 ; then
    AC_DEFINE(HAVE_LONG_LONG_INT,1,[Define if C has long long int])
fi
])dnl
dnl
dnl The AC_C_LONG_DOUBLE([]) macro is junk because it assumes that the
dnl long double type is valid in the language!  In other words, that
dnl the compiler is ANSI C
dnl
AC_DEFUN([PAC_LONG_DOUBLE],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_MSG_CHECKING([for long double])
if test -n "$GCC"; then
AC_DEFINE(HAVE_LONG_DOUBLE,1,[Define if the 'long double' type works.])
AC_MSG_RESULT(yes)
else
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[long double a;return 0;]])],
[ldok=1],[ldok=0])

if test $ldok = 1 ; then
AC_RUN_IFELSE(AC_LANG_PROGRAM(,[[
/* On Ultrix 4.3 cc, long double is 4 and double is 8.  */
exit(sizeof(long double) < sizeof(double)); ]]),
  AC_DEFINE(HAVE_LONG_DOUBLE,1,[Define if the 'long double' type works.])
  AC_MSG_RESULT(yes),
  AC_MSG_RESULT(no))
else
  AC_MSG_RESULT(no)
fi
fi
])dnl
dnl
dnl PAC_HAVE_VOLATILE
dnl 
dnl Defines HAS_VOLATILE if the C compiler accepts "volatile" 
dnl
AC_DEFUN([PAC_HAVE_VOLATILE],
[AC_MSG_CHECKING([for volatile])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[volatile int a;]], [[main();]])],
[AC_DEFINE(HAS_VOLATILE,1,Define is C supports volatile declaration)
AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

])dnl
dnl
dnl
dnl
AC_DEFUN([PAC_WORDS_BIGENDIAN],
[AC_MSG_CHECKING([byte ordering])
AC_RUN_IFELSE(AC_LANG_PROGRAM(,[[
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  exit (u.c[sizeof (long) - 1] == 1);
]]), ,pac_r=1)
if test -z "$pac_r" ; then
    AC_MSG_RESULT(little endian)
else
    AC_MSG_RESULT(big endian)
    AC_DEFINE(WORDS_BIGENDIAN,1,
	[Define if your processor stores words with the most significant
	byte first (like Motorola and SPARC, unlike Intel and VAX).])
fi
])dnl
dnl
dnl Look for a non-standard library by looking in some named places.
dnl Check for both foo.a and libfoo.a (and .so)
dnl 
dnl PAC_FIND_USER_LIB(LIB-NAME[,LIB-LIST,ACTION-IF-FOUND,ACTION-IF-NOT-FOUND])
dnl (use foo to check for foo.a and libfoo.a)
dnl Checks the usual places, as well as /usr/local/LIBNAME and
dnl /usr/local/LIBNAME/lib .
dnl The location of the library may be found in pac_lib_file.
dnl The DIRECTORY of the library may be found in pac_lib_dir
dnl
AC_DEFUN([PAC_FIND_USER_LIB],
[
AC_MSG_CHECKING([for library $1])
pac_lib_file=""
pac_lib_dir=""
for dir in $2 \
    /usr \
    /usr/local \
    /usr/local/$1 \
    /usr/contrib \
    /usr/contrib/$1 \
    $HOME/$1 \
    /opt/$1 \
    /opt/local \
    /opt/local/$1 \
    /local/encap/$1 ; do
    for suffix in a so ; do
        if test -n "$pac_lib_dir" ; then break ; fi
        if test -r $dir/$1.$suffix ; then
	    pac_lib_file=$dir/$1.$suffix
            pac_lib_dir=$dir
	    break
        fi
        if test -r $dir/lib$1.$suffix ; then
	    pac_lib_file=$dir/lib$1.$suffix
            pac_lib_dir=$dir
	    break
        fi
        if test -r $dir/lib/$1.$suffix ; then
	    pac_lib_file=$dir/lib/$1.$suffix
            pac_lib_dir=$dir/lib
	    break
        fi
        if test -r $dir/lib/lib$1.$suffix ; then
	    pac_lib_file=$dir/lib/lib$1.$suffix
            pac_lib_dir=$dir/lib
	    break
        fi
    done
done
if test -n "$pac_lib_file" ; then 
  AC_MSG_RESULT(found $pac_lib_file)
  ifelse([$3],,,[$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4],,,[$4])
fi
])dnl
dnl
dnl Look for a non-standard include by looking in some named places.
dnl Check for foo.h
dnl 
dnl PAC_FIND_USER_INCLUDE(FILE-NAME[,DIR-LIST,ACTION-IF-FOUND,ACTION-IF-NOT-FOUND])
dnl (use foo to check for foo.h)
dnl Checks the usual places, as well as /usr/local/FILENAME and
dnl /usr/local/FILENAME/include .
dnl The location of the include directory library may be found in pac_inc_dir.
dnl
AC_DEFUN([PAC_FIND_USER_INCLUDE],
[
AC_MSG_CHECKING([for include directory for $1])
pac_inc_dir=""
for dir in $2 \
    /usr \
    /usr/local \
    /usr/local/$1 \
    /usr/contrib \
    /usr/contrib/$1 \
    $HOME/$1 \
    /opt/$1 \
    /opt/local \
    /opt/local/$1 \
    /local/encap/$1 ; do
    if test -r $dir/$1.h ; then
	pac_inc_dir=$dir
	break
    fi
    if test -r $dir/include/$1.h ; then
	pac_inc_dir=$dir/include
	break
    fi
    if test -r $dir/lib/lib$1.a ; then
	pac_lib_file=$dir/lib/lib$1.a
	break
    fi
    if test -r $dir/lib/lib$1.so ; then
	pac_lib_file=$dir/lib/lib$1.so
	break
    fi
done
if test -n "$pac_inc_dir" ; then 
  AC_MSG_RESULT(found $pac_inc_dir)
  ifelse([$3],,,[$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4],,,[$4])
fi
])dnl
dnl
dnl WARNING: AC_HEADER_STDC uses CPP instead of CC!
dnl
dnl
dnl Check to see if malloc is declared as char *malloc or void *malloc
dnl If stdlib.h is not defined, then this will choose char*malloc.
dnl
AC_DEFUN([PAC_MALLOC_RETURNS_VOID],
[AC_MSG_CHECKING(for malloc return type)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>]], [[extern void *malloc();]])],
[eval "ac_cv_malloc=void"],[eval "ac_cv_malloc=char"])
AC_MSG_RESULT($ac_cv_malloc)
if test "$ac_cv_malloc" = void ; then
    AC_DEFINE(MALLOC_RET_VOID,1,
	[Define if malloc returns void * (and is an error to return char *)])
fi
])dnl
dnl
dnl Check that the compile actually runs.  Perform first arg is yes,
dnl second if false
dnl PAC_CHECK_COMPILER_OK(true-action, false-action)
dnl
AC_DEFUN([PAC_CHECK_COMPILER_OK],
[
AC_MSG_CHECKING(that the compiler $CC runs)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[return 0;]])],
[eval "ac_cv_ccworks=yes"],[eval "ac_cv_ccworks=no"])
AC_MSG_RESULT($ac_cv_ccworks)
if test $ac_cv_ccworks = "yes" ; then
    ifelse([$1],,:,[$1])
else
# Generate output from failed test.  See COMPILE_CHECK code
# It really would be better if the compile tests put the output into
# a file for later analysis, like conftest.out
#
cat > conftest.c <<EOF
#include "confdefs.h"
int main() { exit(0); }
int t() { return 0; }
EOF
${CC-cc} $CFLAGS conftest.c -o conftest $LIBS
rm -f conftest* 
#
# End of output
    ifelse([$2],,:,[$2])
fi
])dnl
dnl
dnl Check that the compile accepts ANSI prototypes.  Perform first arg if yes,
dnl second if false.  Only test if it hasn't been tested for this compiler
dnl (and flags) before
dnl PAC_CHECK_CC_PROTOTYPES(true-action, false-action)
dnl
AC_DEFUN([PAC_CHECK_CC_PROTOTYPES],
[
if test "$ac_cv_ccansi" != "$CC $CFLAGS" ; then
AC_MSG_CHECKING(that the compiler $CC accepts ANSI prototypes)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[int f(double a){return 0;}]], [[]])],
[eval "ac_cv_ccworks=yes"],[eval "ac_cv_ccworks=no"])
AC_MSG_RESULT($ac_cv_ccworks)
if test $ac_cv_ccworks = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
ac_cv_ccansi="$CC $CFLAGS"
fi
])dnl
dnl
dnl Check that the compile accepts ANSI const type.  Perform first arg if yes,
dnl second if false
dnl PAC_CHECK_CC_CONST(true-action, false-action)
dnl
AC_DEFUN([PAC_CHECK_CC_CONST],
[
AC_MSG_CHECKING(that the compiler $CC accepts const modifier)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[int f(const int a){return a;}]], [[]])],
[eval "ac_cv_ccworks=yes"],[eval "ac_cv_ccworks=no"])
AC_MSG_RESULT($ac_cv_ccworks)
if test $ac_cv_ccworks = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
])dnl
dnl
dnl Check that the compile accepts ANSI CPP concatenation.  Perform first 
dnl arg if yes, second if false
dnl PAC_CHECK_CPP_CONCAT(true-action, false-action)
dnl
AC_DEFUN([PAC_CHECK_CPP_CONCAT],
[
ac_pound="#"
AC_MSG_CHECKING([that the compiler $CC accepts $ac_pound$ac_pound for concatenation in cpp])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#define concat(a,b) a##b]], [[int concat(a,b);return ab;]])],
[eval "ac_cv_ccworks=yes"],[eval "ac_cv_ccworks=no"])
AC_MSG_RESULT($ac_cv_ccworks)
if test $ac_cv_ccworks = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
])dnl
dnl
dnl Test the compiler to see if it actually works.  First, check to see
dnl if the compiler works at all
dnl Uses TESTCC, not CC
dnl 
dnl The test directory is ccbugs by default, but can be overridded with 
dnl CCBUGS
dnl
AC_DEFUN([PAC_CORRECT_COMPILER],
[
if test -z "$CCBUGS" ; then CCBUGS=$srcdir/ccbugs ; fi
if test -d $CCBUGS ; then 
    # Use "LTESTCC" as "local Test CC"
    if test -z "$TESTCC" ; then LTESTCC="$CC" ; else LTESTCC="$TESTCC" ; fi
    for file in $CCBUGS/ccfail*.c ; do
        CFILE=`basename $file .c`
	msg=`cat $CCBUGS/$CFILE.title`
        AC_MSG_CHECKING([$msg])
        cp $file conftest.c
        broken=1
        rm -f conftest.out conftest.rout
        if eval $LTESTCC $CFLAGS -o conftest conftest.c $LIBS >conftest.out 2>&1 ; then
	    if test -s conftest ; then
                ./conftest 2>&1 1>conftest.rout
                if test $? = 0 ; then
  	            broken=0
                fi
	    fi
        fi
        if test $broken = 1 ; then
	    AC_MSG_RESULT(no)
	    cat $CCBUGS/$CFILE.txt | sed 's/^/\*\#/g'
	    if test -s conftest.out ; then
	        echo "Output from compile step was:"
		cat conftest.out
	    fi
	    if test -s conftest.rout ; then
	        echo "Output from run step was:"
		cat conftest.rout
	    fi
	else
	    AC_MSG_RESULT(yes)
        fi
	/bin/rm -f conftest conftest.c conftest.o conftest.out conftest.rout
    done
    #
    # These are non-fatal, but must be run
    for file in $CCBUGS/ccnfail*.c ; do
        CFILE=`basename $file .c`
	msg=`cat $CCBUGS/$CFILE.title`
        AC_MSG_CHECKING([$msg])
        cp $file conftest.c
        nbroken=1
	rm -f conftest.out conftest.rout
        if eval $LTESTCC $CFLAGS -o conftest conftest.c $LIBS >conftest.out 2>&1 ; then
	    if test -s conftest ; then
                ./conftest 2>&1 1>conftest.rout
                if test $? = 0 ; then
                    nbroken=0
                fi
	    fi
        fi
        if test $nbroken = 1 ; then 
	    AC_MSG_RESULT(no)
	    cat $CCBUGS/$CFILE.txt | sed 's/^/\*\#/g'
	    if test -s conftest.out ; then
	        echo "Output from compile step was:"
		cat conftest.out
	    fi
	    if test -s conftest.rout ; then
	        echo "Output from run step was:"
		cat conftest.rout
	    fi
	else
	    AC_MSG_RESULT(yes)
        fi
	/bin/rm -f conftest conftest.c conftest.o conftest.out conftest.rout
    done

    #
    # Now, try the warnings.  Note that this just does compiles, not runs
    for file in $CCBUGS/ccwarn*.c ; do
        CFILE=`basename $file .c`
	msg=`cat $CCBUGS/$CFILE.title`
        AC_MSG_CHECKING([$msg])
        cp $file conftest.c
	echo "${CC-cc} $CFLAGS ... test for quotes in [defn]" >>config.log
        if eval ${CC-cc} $CFLAGS \
	    -DCONFIGURE_ARGS_CLEAN="'"'"'-A -B'"'"'" -c \
	    conftest.c $LIBS >> config.log 2>&1 ; then
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	    cat $CCBUGS/$CFILE.txt | sed 's/^/\*\#/g' 
	    if test "$CFILE" = "ccwarn1" ; then
	       CONFIGURE_ARGS_CLEAN="`echo $CONFIGURE_ARGS_CLEAN | tr ' ' '_'`"
            fi
        fi
	# set +x
	/bin/rm -f conftest conftest.[[co]]
    done
    #
    # After everything, see if there are any problems
    if test $broken = 1 ; then 
        if test -z "$FAILMSG" ; then
	    echo "Compiler $CC appears broken; aborting configure..."
        else
	    eval echo "$FAILMSG"
        fi
        exit 1
    fi
fi
])dnl
dnl
dnl Check that the Fortran compiler works.  We needed this first for LINUX
dnl Perform first arg is yes, second if false
dnl PAC_CHECK_F77_COMPILER_OK(true-action, false-action)
dnl The name of the compiler is F77
dnl
AC_DEFUN([PAC_CHECK_F77_COMPILER_OK],
[
AC_MSG_CHECKING(that the compiler $F77 runs)
cat >conftest.f <<EOF
          program main
          end
EOF
/bin/rm -f conftest.out
$F77 $FFLAGS -c conftest.f > conftest.out 2>&1
if test $? != 0 ; then
    AC_MSG_RESULT(no)
    echo "Fortran compiler returned non-zero return code"
    if test -s conftest.out ; then
	echo "Output from test was"
        cat conftest.out
    fi
    ifelse([$2],,:,[$2])
elif test ! -s conftest.o ; then
    AC_MSG_RESULT(no)
    echo "Fortran compiler did not produce object file"
    if test -s conftest.out ; then
	echo "Output from test was"
        cat conftest.out
    fi
    ifelse([$2],,:,[$2])
else    
    AC_MSG_RESULT(yes)
    ifelse([$1],,:,[$1])
fi
rm -f conftest* 
])dnl
dnl
dnl
dnl Check that the Fortran 90 compiler works.  
dnl Perform first arg is yes, second if false
dnl PAC_CHECK_F90_COMPILER_OK(true-action, false-action)
dnl The name of the compiler is F90
dnl Also defines F90SUFFIX as f or f90 (xlf90 accepts only f; Solaris
dnl uses suffix to decide on free versus fixed format.
dnl
AC_DEFUN([PAC_CHECK_F90_COMPILER_OK],
[
AC_MSG_CHECKING(that the compiler $F90 runs)
cat >conftest.f <<EOF
          program main
          end
EOF
/bin/rm -f conftest.out
$F90 $F90FLAGS -c conftest.f > conftest.out 2>&1
if test $? != 0 ; then
    AC_MSG_RESULT(no)
    echo "Fortran 90 compiler returned non-zero return code"
    if test -s conftest.out ; then
	echo "Output from test was"
        cat conftest.out
    fi
    ifelse([$2],,:,[$2])
elif test ! -s conftest.o ; then
    AC_MSG_RESULT(no)
    echo "Fortran 90 compiler did not produce object file"
    if test -s conftest.out ; then
	echo "Output from test was"
        cat conftest.out
    fi
    ifelse([$2],,:,[$2])
else    
    AC_MSG_RESULT(yes)
    ifelse([$1],,:,[$1])
    /bin/mv -f conftest.f conftest.f90
    AC_MSG_CHECKING(that the compiler $F90 accepts .f90 suffix)
    $F90 $F90FLAGS -c conftest.f90 > conftest.out 2>&1
    if test $? != 0 ; then
        F90SUFFIX="f"
	AC_MSG_RESULT(no)
    else
        F90SUFFIX="f90"
	AC_MSG_RESULT(yes)
    fi
fi
rm -f conftest* 
])dnl
dnl
dnl Look for a style of VPATH.  Known forms are
dnl VPATH = .:dir
dnl .PATH: . dir
dnl
dnl Defines VPATH or .PATH with . $(srcdir)
dnl Requires that vpath work with implicit targets
dnl NEED TO DO: Check that $< works on explicit targets.
dnl
AC_DEFUN([PAC_MAKE_VPATH],[
AC_SUBST(VPATH)
AC_MSG_CHECKING(for virtual path format)
rm -rf conftest*
mkdir conftestdir
cat >conftestdir/a.c <<EOF
A sample file
EOF
cat > conftest <<EOF
all: a.o
VPATH=.:conftestdir
.c.o:
	@echo \$<
EOF
ac_out=`$MAKE -f conftest 2>&1 | grep 'conftestdir/a.c'`
if test -n "$ac_out" ; then 
    AC_MSG_RESULT(VPATH)
    VPATH='VPATH=.:$(srcdir)'
else
    rm -f conftest
    cat > conftest <<EOF
all: a.o
.PATH: . conftestdir
.c.o:
	@echo \$<
EOF
    ac_out=`$MAKE -f conftest 2>&1 | grep 'conftestdir/a.c'`
    if test -n "$ac_out" ; then 
        AC_MSG_RESULT(.PATH)
        VPATH='.PATH: . $(srcdir)'
    else
	AC_MSG_RESULT(neither VPATH nor .PATH works)
    fi
fi
rm -rf conftest*
])dnl
dnl
dnl Here begins macros for setting defaults for specific systems.
dnl These handle things like C compilers with funny names and special
dnl options.
dnl
dnl Fortran runtime for Fortran/C linking
dnl On suns, try
dnl FC_LIB          =/usr/local/lang/SC2.0.1/libM77.a \ 
dnl              /usr/local/lang/SC2.0.1/libF77.a -lm \
dnl              /usr/local/lang/SC2.0.1/libm.a \
dnl              /usr/local/lang/SC2.0.1/libansi.a
dnl
dnl AIX requires -bI:/usr/lpp/xlf/lib/lowsys.exp
dnl ------------------------------------------------------------------------
dnl
dnl Get the format of Fortran names.  Uses F77, FFLAGS, and sets WDEF.
dnl
AC_DEFUN([PAC_GET_FORTNAMES],[
   # Check for strange behavior of Fortran.  For example, some FreeBSD
   # systems use f2c to implement f77, and the version of f2c that they 
   # use generates TWO (!!!) trailing underscores
   # Currently, WDEF is not used but could be...
   #
   # Eventually, we want to be able to override the choices here and
   # force a particular form.  This is particularly useful in systems
   # where a Fortran compiler option is used to force a particular
   # external name format (rs6000 xlf, for example).
   cat > conftest.f <<EOF
       subroutine mpir_init_fop( a )
       integer a
       a = 1
       return
       end
EOF
   echo "$F77 $FFLAGS -c conftest.f" >> config.log
   $F77 $FFLAGS -c conftest.f >> config.log 2>&1
   if test ! -s conftest.o ; then
	AC_MSG_WARN([Unable to test Fortran compiler (compiling a test program
		     failed to produce an object file).])
	F77=""
   elif test -z "$FORTRANNAMES" ; then
    # We have to be careful here, since the name may occur in several
    # forms.  We try to handle this by testing for several forms
    # directly.
    if test $arch_CRAY ; then
     # Cray doesn't accept -a ...
     nameform1=`strings conftest.o | grep mpir_init_fop_  | sed -n -e '1p'`
     nameform2=`strings conftest.o | grep MPIR_INIT_FOP   | sed -n -e '1p'`
     nameform3=`strings conftest.o | grep mpir_init_fop   | sed -n -e '1p'`
     nameform4=`strings conftest.o | grep mpir_init_fop__ | sed -n -e '1p'`
    else
     nameform1=`strings -a conftest.o | grep mpir_init_fop_  | sed -n -e '1p'`
     nameform2=`strings -a conftest.o | grep MPIR_INIT_FOP   | sed -n -e '1p'`
     nameform3=`strings -a conftest.o | grep mpir_init_fop   | sed -n -e '1p'`
     nameform4=`strings -a conftest.o | grep mpir_init_fop__ | sed -n -e '1p'`
    fi
    rm -f conftest.f conftest.o
    if test -n "$nameform4" ; then
	echo "Fortran externals are lower case and have 1 or 2 trailing underscores"
	FORTRANNAMES="FORTRANDOUBLEUNDERSCORE"
    elif test -n "$nameform1" ; then
        # We don't set this in CFLAGS; it is a default case
        echo "Fortran externals have a trailing underscore and are lowercase"
	FORTRANNAMES="FORTRANUNDERSCORE"
    elif test -n "$nameform2" ; then
	echo "Fortran externals are uppercase"     
	FORTRANNAMES="FORTRANCAPS" 
    elif test -n "$nameform3" ; then
	echo "Fortran externals are lower case"
	FORTRANNAMES="FORTRANNOUNDERSCORE"
    else
	AC_MSG_WARN(Unable to determine the form of Fortran external names.
		    Make sure that the compiler $F77 can be run on this system.
		    Turning off Fortran.)
	F77=""
    fi
    fi
    if test -n "$FORTRANNAMES" ; then
        WDEF="-D$FORTRANNAMES"
    fi
    ])dnl
dnl
dnl ------------------------------------------------------------------------
dnl
dnl AR = name with args to create/add to archive
dnl As input, ARLOCAL is the arg that should be use for using the local 
dnl directory
dnl ARNAME = just name
dnl ARARGS = just args
AC_DEFUN([PAC_GET_AR],
[
if test -z "$USERAR" ; then
case $1 in 
   intelnx|paragon|i860) AR="ar860 crl" ; ARNAME="ar860" ; ARARGS="crl" ;;
   cm5) AR="ar cr" ; ARNAME="ar" ; ARARGS="cr"
   ;;
   meiko|solaris) AR="ar cr" ; ARNAME="ar" ; ARARGS="cr" 
   ;;
   ncube) AR="nar cr" ; ARNAME="nar" ; ARARGS="cr" ;;
esac
fi
if test -z "$AR" ; then 
    AR="ar cr$ARLOCAL" ; ARNAME="ar" ; ARARGS="cr$ARLOCAL" 
fi
])dnl
dnl --------------------------------------------------------
dnl Test for the VERSION of tk.  There are major changes between 3.6 and 4.0
dnl (in particular, the type Tk_ColorModel disappeared
dnl  Put result into TK_VERSION (as, e.g., 3.6 or 4.0).  Should test version
dnl as STRING, since we don't control the changes between versions, and 
dnl only versions that we know should be tested.
dnl Note that this may be important ONLY if you include tk.h .
dnl TKINCDIR may also be defined if the include files are not where the
dnl architecture-dependant library files are
dnl
dnl TK_LIB and XINCLUDES must be defined (and no_x must NOT be true)
dnl
AC_DEFUN([PAC_TK_VERSION],
[
AC_MSG_CHECKING(for version of TK)
/bin/rm -f conftestval
#
# Some systems have a separate tcl dir; since we need both tcl and tk
# we include both directories
# Tk is going to load X11; if no X11, skip this step
if test -z "$no_x" -a -n "$TK_DIR" -a -n "$TCL_DIR" ; then
  CFLAGSsave="$CFLAGS"
  CFLAGS="$CFLAGS -I$TK_DIR/include -I$TCL_DIR/include $XINCLUDES"
  if test -n "$TKINCDIR" ; then
      CFLAGS="$CFLAGS -I$TKINCDIR/include"
  fi
  AC_RUN_IFELSE(AC_LANG_PROGRAM([[#include "tk.h"
#include <stdio.h>]],[[
FILE *fp = fopen( "conftestval", "w" ); 
fprintf( fp, "%d.%d", TK_MAJOR_VERSION, TK_MINOR_VERSION );
return 0;]]),
  TK_VERSION=`cat conftestval`,
  TK_VERSION="unavailable",
  TK_VERSION="unavailable")
  CFLAGS="$CFLAGSsave"
elif test -n "$wishloc" ; then
  # It is possible to use a wish program with
  # set tk_version [ string range $tk_patchLevel 0 2 ]
  # puts stdout $tk_version
  TK_VERSION="unavailable"
else
  TK_VERSION="unavailable"
fi
AC_MSG_RESULT($TK_VERSION)
])dnl
dnl
dnl This version compiles an entire function; used to check for
dnl things like varargs done correctly
dnl
dnl PAC_COMPILE_CHECK_FUNC(msg,function,if_true,if_false)
dnl
AC_DEFUN([PAC_COMPILE_CHECK_FUNC],
[AC_PROVIDE([$0])dnl
ifelse([$1], , , [AC_MSG_CHECKING(for $1)]
)dnl
if test ! -f confdefs.h ; then
    AC_MSG_RESULT("!! SHELL ERROR !!")
    echo "The file confdefs.h created by configure has been removed"
    echo "This may be a problem with your shell; some versions of LINUX"
    echo "have this problem.  See the Installation guide for more"
    echo "information."
    exit 1
fi
cat > conftest.c <<EOF
#include "confdefs.h"
[$2]
EOF
dnl Don't try to run the program, which would prevent cross-configuring.
if test -z "$ac_compile_link" ; then 
    ac_compile_link='${CC-cc} $CFLAGS conftest.c -o conftest $LIBS >>config.log 2>&1'
fi
echo "$ac_compile_link" >>config.log
cat conftest.c >>config.log
if eval $ac_compile_link; then
  ifelse([$1], , , [AC_MSG_RESULT(yes)])
  ifelse([$3], , :, [rm -rf conftest*
  $3
])
else
    rm -rf conftest*
    ifelse([$4], , , $4)
    ifelse([$1], , , [AC_MSG_RESULT(no)])
fi
rm -f conftest*]
)dnl
dnl
dnl Append SH style definitions to a file
dnl To generate a site file (for MAKE), use PAC_APPEND_FILE.  This allows
dnl you to use configure to create a likely site file.
dnl
dnl PAC_APPEND_FILE(varname,varvalue,file)
dnl Example: PAC_APPEND_FILE("CC",$CC,"make.site")
dnl
AC_DEFUN([PAC_APPEND_FILE],
[
if test "$3" = "-" ; then echo "$1=$2" ; else echo "$1=$2" >> $3 ; fi
])dnl
dnl
dnl See if Fortran compiler accepts -Idirectory flag
dnl 
dnl PAC_FORTRAN_HAS_INCDIR(directory,true-action,false-action)
dnl
dnl Fortran compiler is F77 and is passed FFLAGS
dnl
AC_DEFUN([PAC_FORTRAN_HAS_INCDIR],
[
AC_MSG_CHECKING([for Fortran include argument])
cat > $1/conftestf.h <<EOF
       call sub()
EOF
cat > conftest.f <<EOF
       program main
       include 'conftestf.h'
       end
EOF
echo "$F77 $FFLAGS -c -I$1 conftest.f" >> config.log
if $F77 $FFLAGS -c -I$1 conftest.f >> config.log 2>&1 ; then
    FINCARG="-I"
    ifelse($2,,true,$2)
    AC_MSG_RESULT([supports -I for include])
elif $F77 $FFLAGS -c -Wf,-I$1 conftest.f >> config.log 2>&1 ; then
    FINCARG="-Wf,-I"
    ifelse($2,,true,$2)
    AC_MSG_RESULT([supports -Wf,-I for include])
else
    ifelse($3,,true,$3)
    AC_MSG_RESULT([does NOT support -I or -Wf,-I for include])
fi
/bin/rm -f conftest.f $1/conftestf.h
])dnl
dnl
dnl PAC_FORTRAN_GET_INTEGER_SIZE(var_for_size)
dnl
dnl sets var_for_size to the size.  Ignores if the size cannot be determined
dnl
AC_DEFUN([PAC_FORTRAN_GET_INTEGER_SIZE],
[AC_MSG_CHECKING([for size of Fortran INTEGER])
/bin/rm -f conftestval
/bin/rm -f conftestf.f conftestf.o
cat <<EOF > conftestf.f
      subroutine isize( )
      integer i(2)
      call cisize( i(1), i(2) )
      end
EOF
echo "$F77 $FFLAGS -c conftestf.f" >>config.log
if $F77 $FFLAGS -c conftestf.f >>config.log 2>&1 ; then 
    SaveLIBS="$LIBS"
    LIBS="conftestf.o $LIBS"
    AC_RUN_IFELSE(AC_LANG_PROGRAM([[#include <stdio.h>
#ifdef FORTRANCAPS
#define cisize_ CISIZE
#define isize_  ISIZE
#elif defined(FORTRANNOUNDERSCORE)
#define cisize_ cisize
#define isize_  isize
#endif
static int isize_val;
void cisize_( i1p, i2p )
char *i1p, *i2p;
{
	isize_val = (i2p - i1p) * sizeof(char);
}]],
[[FILE *f=fopen("conftestval","w");
  
  if (!f) exit(1);
  isize_();
  fprintf( f, "%d\n", isize_val);
  exit(0);
]]),Pac_CV_NAME=`cat conftestval`,
Pac_CV_NAME="",
Pac_CV_NAME="")
LIBS="$SaveLIBS"
fi
if test -z "$Pac_CV_NAME" ; then
    # Try to compile/link with the Fortran compiler instead.  This
    # worked for the NEC SX-4
    compile_f='${CC-cc} $CFLAGS -c conftest.c; ${F77-f77} $FFLAGS -o conftest conftest.o $LIBS >config.log 2>&1'
    echo "$compile_f" >> config.log
    eval $compile_f
    if test ! -s conftest ; then 
	echo "Could not build executable program:"
	echo "${F77-f77} $FFLAGS -o conftest conftest.o $LIBS"
    else
	/bin/rm -f conftestout
	if test -s conftest && (./conftest;exit) 2>conftestout ; then
	    Pac_CV_NAME=`cat conftestval`
        fi
    fi

fi
/bin/rm -f conftestf.f conftestf.o
if test -n "$Pac_CV_NAME" -a "$Pac_CV_NAME" != 0 ; then
    AC_MSG_RESULT($Pac_CV_NAME)
else
    AC_MSG_RESULT(unavailable)
fi
$1=$Pac_CV_NAME
])dnl
dnl
dnl PAC_FORTRAN_GET_REAL_SIZE(var_for_size)
dnl
dnl sets var_for_size to the size.  Ignores if the size cannot be determined
dnl
AC_DEFUN([PAC_FORTRAN_GET_REAL_SIZE],
[AC_MSG_CHECKING([for size of Fortran REAL])
/bin/rm -f conftestval
/bin/rm -f conftestf.f conftestf.o
cat <<EOF > conftestf.f
      subroutine isize( )
      real i(2)
      call cisize( i(1), i(2) )
      end
EOF
echo "$F77 $FFLAGS -c conftestf.f" >>config.log
if $F77 $FFLAGS -c conftestf.f >>config.log 2>&1 ; then 
    SaveLIBS="$LIBS"
    LIBS="conftestf.o $LIBS"
    AC_RUN_IFELSE(AC_LANG_PROGRAM([[#include <stdio.h>
#ifdef FORTRANCAPS
#define cisize_ CISIZE
#define isize_  ISIZE
#elif defined(FORTRANNOUNDERSCORE)
#define cisize_ cisize
#define isize_  isize
#endif
static int isize_val;
void cisize_( i1p, i2p )
char *i1p, *i2p;
{
	isize_val = (i2p - i1p) * sizeof(char);
}]],
[[FILE *f=fopen("conftestval","w");
  
  if (!f) exit(1);
  isize_();
  fprintf( f, "%d\n", isize_val);
  exit(0);
]]),Pac_CV_NAME=`cat conftestval`,
Pac_CV_NAME="",
Pac_CV_NAME="")
LIBS="$SaveLIBS"
fi
/bin/rm -f conftestf.f conftestf.o
if test -n "$Pac_CV_NAME" -a "$Pac_CV_NAME" != 0 ; then
    AC_MSG_RESULT($Pac_CV_NAME)
else
    AC_MSG_RESULT(unavailable)
fi
$1=$Pac_CV_NAME
])dnl
dnl
dnl See if Fortran accepts ! for comments
dnl
dnl PAC_FORTRAN_HAS_EXCLAM_COMMENTS(action-if-true,action-if-false)
dnl
AC_DEFUN([PAC_FORTRAN_HAS_EXCLAM_COMMENTS],
[
AC_MSG_CHECKING([for Fortran accepts ! for comments])
cat > conftest.f <<EOF
       program main
!      This is a comment
       end
EOF
echo "$F77 $FFLAGS -c conftest.f" >>config.log
if $F77 $FFLAGS -c conftest.f >> config.log 2>&1 ; then
    ifelse($1,,true,$1)
    AC_MSG_RESULT([yes])
else
    ifelse($2,,true,$2)
    AC_MSG_RESULT([no])
fi
/bin/rm -f conftest.f
])dnl
dnl
dnl tries to determine the Fortran 90 kind parameter for 8-byte integers
dnl
dnl Set decimal digits to 2 for int*1, 4 for int*2, 8 (or 9) for int*4, and
dnl 16 to 18 for int*8.  If not set, it assumes 16.
dnl PAC_FORTRAN_INT_KIND([variable to set to kind value],[decimal digits])
dnl The value is -1 if it is not available
dnl The second arg is the number of BYTES
AC_DEFUN([PAC_FORTRAN_INT_KIND],
[
AC_MSG_CHECKING([for Fortran 90 KIND parameter for ifelse($2,,8-byte,$2-byte) integers])
# We need to evaluate the second arg, which may be a runtime value
sellen="$2"
if test -z "$sellen" ; then 
    sellen=16
else 
    # Convert bytes to digits
    case $sellen in 
	1) sellen=2 ;;
	2) sellen=4 ;;
	4) sellen=8 ;;
	8) sellen=16 ;;
	16) sellen=30 ;;
        *) sellen=8 ;;
    esac
fi
rm -f conftest*
cat <<EOF > conftest.f
      program main
      integer i
      i = selected_int_kind($sellen)
      open(8, file="conftest1.out", form="formatted")
      write (8,*) i
      close(8)
      stop
      end
EOF
if test -z "$F90" ; then
   F90=f90
fi
KINDVAL=""
# We must be careful in case the F90LINKER isn't the same as F90
# (e.g., it has extra options or is a different program)
if $F90 -c -o conftest.o conftest.f >conftest.out 2>&1 ; then
    # Use F90 if we can (in case the linker prepares programs
    # for a parallel environment).
    echo "$F90 -o conftest conftest.o" >> config.log
    if $F90 -o conftest conftest.o >>config.log 2>&1 ; then
	F90LINKERTEST="$F90" 
    elif test -z "$F90LINKER" ; then 
	F90LINKERTEST="$F90"
    else
	F90LINKERTEST="$F90LINKER"
    fi
    if $F90LINKERTEST -o conftest conftest.o >conftest.out 2>&1 ; then
        ./conftest >>conftest.out 2>&1
        if test -s conftest1.out ; then
	    # Because of write, there may be a leading blank.
            KINDVAL=`cat conftest1.out | sed 's/ //g'`
        else
	    if test -s conftest.out ; then cat conftest.out >> config.log ; fi
	    KINDVAL=-1
        fi
    else
        echo "Failure to link program to test for INTEGER kind" >>config.log
        $F90LINKER -o conftest conftest.f >>config.log 2>&1
    fi
else 
   echo "Failure to build program to test for INTEGER kind" >>config.log
   $F90 -o conftest conftest.f >>config.log 2>&1
fi
rm -f conftest*
if test -n "$KINDVAL" -a "$KINDVAL" != "-1" ; then
   AC_MSG_RESULT($KINDVAL)
   ifelse($1,,,$1=$KINDVAL)
else
   AC_MSG_RESULT(unavailable)
   ifelse($1,,,$1="-1")
fi
])dnl
dnl
dnl
dnl Check that signal semantics work correctly
dnl
AC_DEFUN([PAC_SIGNALS_WORK],
[
AC_MSG_CHECKING([that signals work correctly])
cat >conftest.c <<EOF
#include <signal.h>
static int rc = 0, irc = 1, maxcnt=5;
void handler( sig )
int sig;
{
void (*oldsig)();
oldsig = signal( SIGUSR1, handler );
if (oldsig != handler) rc = 1;
irc = 0;
}
int main(argc, argv)
int argc;
char **argv;
{
(void)signal( SIGUSR1, handler );
kill( getpid(), SIGUSR1 );
while (irc && maxcnt) { sleep(1); maxcnt--;}
return rc;
}
EOF
rm -f conftest.out
if eval ${CC-cc} $CFLAGS -o conftest conftest.c > conftest.out 2>&1 ; then
    if ./conftest ; then
	AC_MSG_RESULT(yes)
    else
	if test -s conftest.out ; then cat conftest.out >> config.log ; fi
        cat conftest.c >>config.log
	AC_MSG_RESULT(Signals reset when used!)
	AC_DEFINE(SIGNALS_RESET_WHEN_USED,1,
		[Define if signals reset to the default when used (SYSV vs BSD
		semantics). Such signals are essentially un-usable, because of
		the resulting race condition. The fix is to use the sigaction
		etc. routines instead (they're usually available, since without
		them signals are entirely useless)])
    fi
else
    if test -s conftest.out ; then cat conftest.out >> config.log ; fi
    cat conftest.c >>config.log
    AC_MSG_RESULT(Could not compile test program!)
fi
/bin/rm -f conftest conftest.c conftest.o conftest.out
])dnl
dnl
dnl
dnl record top-level directory (this one)
dnl A problem.  Some systems use an NFS automounter.  This can generate
dnl paths of the form /tmp_mnt/... . On SOME systems, that path is
dnl not recognized, and you need to strip off the /tmp_mnt. On others, 
dnl it IS recognized, so you need to leave it in.  Grumble.
dnl The real problem is that OTHER nodes on the same NFS system may not
dnl be able to find a directory based on a /tmp_mnt/... name.
dnl
dnl It is WRONG to use $PWD, since that is maintained only by the C shell,
dnl and if we use it, we may find the 'wrong' directory.  To test this, we
dnl try writing a file to the directory and then looking for it in the 
dnl current directory.  Life would be so much easier if the NFS automounter
dnl worked correctly.
dnl
dnl PAC_GETWD(varname [, filename ] )
dnl 
dnl Set varname to current directory.  Use filename (relative to current
dnl directory) if provided to double check.
dnl
dnl Need a way to use "automounter fix" for this.
dnl
AC_DEFUN([PAC_GETWD],
[
AC_MSG_CHECKING(for current directory name)
$1=$PWD
if test "${$1}" != "" -a -d "${$1}" ; then 
    if test -r ${$1}/.foo$$ ; then
        /bin/rm -f ${$1}/.foo$$
	/bin/rm -f .foo$$
    fi
    if test -r ${$1}/.foo$$ -o -r .foo$$ ; then
	$1=
    else
	echo "test" > ${$1}/.foo$$
	if test ! -r .foo$$ ; then
            /bin/rm -f ${$1}/.foo$$
	    $1=
        else
 	    /bin/rm -f ${$1}/.foo$$
	fi
    fi
fi
if test "${$1}" = "" ; then
    $1=`pwd | sed -e 's%/tmp_mnt/%/%g'`
fi
dnl
dnl First, test the PWD is sensible
ifelse($2,,,
if test ! -r ${$1}/$2 ; then
    dnl PWD must be messed up
    $1=`pwd`
    if test ! -r ${$1}/$2 ; then
	AC_MSG_ERROR(Cannot determine the root directory!)
        exit 1
    fi
    $1=`pwd | sed -e 's%/tmp_mnt/%/%g'`
    if test ! -d ${$1} ; then 
	AC_MSG_WARN(Warning: your default path uses the automounter; this may
		    cause some problems if you use other NFS-connected
		    systems.)
        $1=`pwd`
    fi
fi)
if test -z "${$1}" ; then
    $1=`pwd | sed -e 's%/tmp_mnt/%/%g'`
    if test ! -d ${$1} ; then 
	AC_MSG_WARN(Warning: your default path uses the automounter; this may
		    cause some problems if you use other NFS-connected
		    systems.)
        $1=`pwd`
    fi
fi
AC_MSG_RESULT(${$1})
])dnl
dnl
dnl
dnl 
AC_DEFUN([PAC_GET_SPECIAL_SYSTEM_INFO],
[
#
# We should provide a way to specify a particular IRIX version, rather 
# than requiring the this code to figure everything out.
# In particular, there are IRIX-like systems that do not have the 'hinv'
# command.
#
if test -n "$arch_IRIX" ; then
   AC_MSG_CHECKING(for IRIX OS version)
   dnl Every version and machine under IRIX is incompatible with every other
   dnl version.  This block of code replaces a generic "IRIX" arch value 
   dnl with 
   dnl  IRIX_<version>_<chip>
   dnl  For example
   dnl  IRIX_5_4400 (IRIX 5.x, using MIPS 4400)
   osversion=`uname -r | sed 's/\..*//'`
   dnl Get the second field (looking for 6.1)
   dnl this handles 6.1.27
   dnl   osvminor=`uname -r | sed 's/[0-9]\.\([0-9]*\)\..*/\1/'`
   [osvminor=`uname -r | sed 's/[0-9]\.\([0-9]*\).*/\1/'`]
   AC_MSG_RESULT($osversion)
   dnl Get SGI processor count by quick hack
   dnl 7/13/95, bri@sgi.com
   AC_MSG_CHECKING(for IRIX cpucount)
   [cpucount=`hinv | grep '[0-9]* [0-9]* MHZ IP[0-9]* Proc' | cut -f 1 -d' '`
   if test "$cpucount" = "" ; then
     cpucount=`hinv | grep 'Processor [0-9]*:' | wc -l | sed -e 's/ //g'`
   fi]
   if test "$cpucount" = "" ; then
     AC_MSG_WARN(Could not determine cpucount. Please send)
     hinv
     AC_MSG_ERROR(to mpi-bugs@mcs.anl.gov)
   fi
   AC_MSG_RESULT($cpucount)
   if test -z "$PROCESSOR_COUNT" ; then PROCESSOR_COUNT=$cpucount ; fi
   AC_DEFINE_UNQUOTED(PROCESSOR_COUNT, $PROCESSOR_COUNT,
	[The number of processors expected on an SMP.  Usually undefined])
   dnl
   dnl 
   dnl Check for fast SGI device
   if test -d mpid/sgi -a "$osversion" -ge 6 -a "$osvminor" -ge 1 -a \
	`uname -s` = "IRIX64" ; then
	if test -z "$device_sgi" ; then
	    echo "Consider using -device=sgi for SGI arrays"
	fi
   elif test -n "$device_sgi" ; then
	AC_MSG_ERROR(The sgi device requires IRIX64 and version 6.1 or later)
   fi
   dnl
   dnl Set -comm=shared if IRIX MP & COMM=ch_p4 & COMM not explicitly set
   dnl 7/13/95 bri@sgi.com
   if test $cpucount -gt 1 ; then
     if test "$COMM" = "ch_p4" ; then
       if test "$default_comm" = "1" ; then
         echo "IRIX multiprocessor & p4, setting -comm=shared"
         echo "  (configure with -comm=ch_p4 to disable shared memory)"
         COMM="shared"
       fi
     fi
   fi

   AC_MSG_CHECKING(for IRIX cpumodel)
   dnl The tail -1 is necessary for multiple processor SGI boxes
   dnl We might use this to detect SGI multiprocessors and recommend
   dnl -comm=shared
   cputype=`hinv -t cpu | tail -1 | cut -f 3 -d' '`
   if test -z "$cputype" ; then
	AC_MSG_WARN(Could not get cputype from hinv -t cpu command. Please
		    send)
	hinv -t cpu 2>&1
	hinv -t cpu | cut -f 3 -d' ' 2>&1
	AC_MSG_ERROR(to mpi-bugs@mcs.anl.gov)
   fi
   AC_MSG_RESULT($cputype)
   dnl echo "checking for osversion and cputype"
   dnl cputype may contain R4400, R2000A/R3000, or something else.  
   dnl We may eventually need to look at it.
   if test -z "$osversion" ; then
	AC_MSG_WARN(Could not determine OS version.  Please send)
	uname -a
	AC_MSG_ERROR(to mpi-bugs@mcs.anl.gov)
   elif test $osversion = 4 ; then
	dnl Nathan told us that things worked for IRIX 4 as well; 
	dnl however, we need 'ar ts libname' (ranlib) on version 4 but 
	dnl not the others
        true
   elif test $osversion = 5 ; then
	true
   elif test $osversion = 6 ; then
	true
   else 
       AC_MSG_WARN(Could not recognize the version of IRIX (got $osversion).
       MPICH knows about versions 4, 5 and 6; the version being returned from
       uname -r is $osversion. Please send)
       uname -a 2>&1
       hinv 2>&1
       AC_MSG_ERROR(to mpi-bugs@mcs.anl.gov)
   fi
   AC_MSG_CHECKING(for cputype)
   OLD_ARCH=IRIX
   IRIXARCH="$ARCH_$osversion"
   dnl Now, handle the chip set
   changequote(,)dnl
   cputype=`echo $cputype | sed -e 's%.*/%%' -e 's/R//' | tr -d "[A-Z]"`
   changequote([,])dnl
   case $cputype in 
	3000) ;;
	4000) ;;
	4400) ;;
	4600) ;;
	5000) ;;
	8000) ;;
	10000);;
        *)
	AC_MSG_WARN(Unexpected IRIX/MIPS chipset $cputype. Please send the
		    output)
        uname -a 2>&1
        hinv 2>&1
	AC_MSG_WARN(to mpi-bugs@mcs.anl.gov. MPICH will continue and assume
		    that the cputype is compatible with a MIPS 4400 processor.)
        cputype=4400
	;;
   esac
   AC_MSG_RESULT($cputype)
   IRIXARCH="$IRIXARCH_$cputype"
   AC_MSG_NOTICE(IRIX-specific architecture is $IRIXARCH.)
fi
])dnl
dnl
dnl Check that ranlib works, and is not just a noisy stub
dnl We do this by creating a small object file
dnl and a trial library, and then ranlib the result.
dnl Finally, we try to link with the library (the IRIX Ranlib exists, but
dnl destroys the archive.  User-friendly, it isn't).
dnl
dnl Requires that CC, AR, and RANLIB already be defined.
dnl
AC_DEFUN([PAC_RANLIB_WORKS],
[
AC_MSG_CHECKING(that ranlib works)
broken=0
cat <<EOF >conftest.c
int a(){return 1;}
EOF
rm -f conftest.out
compileonly='${CC-cc} -c $CFLAGS conftest.c >conftest.out 2>&1'
if eval $compileonly ; then 
    :
else
    if test -s conftest.out ; then cat conftest.out >> config.log ; fi
    broken=1;
fi
rm conftest.out
if test $broken = 1 ; then
    AC_MSG_RESULT(no)
    AC_MSG_WARN(Error in creating test object for ranlib!)
else
    arcmd='$AR foo.a conftest.o >conftest.out 2>&1'
    eval $arcmd
    ranlibtest='$RANLIB foo.a >>conftest.out 2>&1'
    if eval $ranlibtest ; then
        : 
    else
	if test -s conftest.out ; then cat conftest.out >> config.log ; fi
        broken=1
    fi
    rm conftest.out
    cat <<EOF >conftest.c
int a(); int main(argc,argv)int argc; char **argv;{ return a();}
EOF
    compileonly='${CC-cc} -c $CFLAGS conftest.c >conftest.out 2>&1'
    if eval $compileonly ; then 
        : 
    else
        broken=1
	if test -s conftest.out ; then cat conftest.out >> config.log ; fi
    fi
    rm conftest.out
    if test $broken = 1 ; then
        AC_MSG_RESULT(no)
        AC_MSG_WARN(Error in creating test program for ranlib test!)
    else
	# Check that we can link the program
	echo $CLINKER $CFLAGS $LDFLAGS conftest.o -o conftest foo.a $LIBS >> \
		config.log
        if eval $CLINKER $CFLAGS $LDFLAGS conftest.o -o conftest foo.a $LIBS \
		>>config.log 2>&1 ; then
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	    AC_MSG_WARN(Error linking with ranlibed library.)
	    broken=1
        fi
    fi
    /bin/rm -f foo.a
    if test $broken = 1 ; then
	AC_MSG_WARN(RANLIB ($RANLIB) failed! Assuming that ranlib is a stub
		returning non-zero condition code.)
        RANLIB=':'
    fi
fi
rm -f conftest.o conftest.c
])dnl
dnl
dnl PAC_FIND_FCLIB( [carch] )
dnl
dnl Find the libraries needed to link Fortran routines with C main programs
dnl This is ONLY an approximation but DOES handle some simple cases.
dnl Sets FCLIB if it can.  Fortran compiler FULL PATH would help.
dnl
AC_DEFUN([PAC_FIND_FCLIB],
[
if test -n "$F77" ; then
PAC_PROGRAM_CHECK(FCVal,$F77,,,FCFULLPATH)
AC_MSG_CHECKING([for Fortran libraries to link C programs with])
case $1 in 
    sun4)
    if test "$FCFULLPATH" = /usr/lang/f77 ; then
	# Look for /usr/lang/SC... .   This is tricky, because 
	# we want to find the LATEST versions first
	for dir in /usr/lang/SC2*.*.* /usr/lang/SC2.* /usr/lang/SC2* \
	 	 /usr/lang/SC1.*.* /usr/lang/SC1.* /usr/lang/SC1* ; do
	    if test -d $dir ; then
		if test -s $dir/libF77.a ; then
		    FCLIB="$dir/libF77.a -lm"
		    if test -s $dir/libm.a ; then
			FCLIB="$FCLIB $dir/libm.a"
		    fi
	            break
	        fi
            fi
        done
    fi
    ;;
    solaris)
	# /opt/SUNWspro/SC*/lib/name
	for file in libF77.a libM77.a libsunmath.a ; do
	    for dir in /opt/SUNWspro/SC4.*/lib /opt/SUNWspro/SC4*/lib \
		       /opt/SUNWspro/SC3.* /opt/SUNWspro/SC3* \
		       /opt/SUNWspro/SC2.* /opt/SUNWspro/SC2* ; do
  	        if test -d $dir ; then
		    if test -s $dir/$file ; then
			FCLIB="$FCLIB $dir/$file"
			break
		    fi
                fi
            done
        done
    ;;

    rs6000)
	for file in /usr/lib/libxlf.a /usr/lib/libxlf90.a ; do
	    if test -s $file ; then
		FCLIB="$FCLIB $file"
            fi
	done
	if test -s /usr/lpp/xlf/lib/lowsys.exp ; then
	    FCLIB="$FCLIB /usr/lpp/xlf/lib/lowsys.exp"
	fi
	;;
    IRIX64|IRIX)
	AC_MSG_RESULT()
        SaveDEFS="$DEFS"
        SaveLIBS="$LIBS"
	AC_CHECK_LIB([fpe],[main],[FCLIB="$FCLIB -lfpe"],[],[])
	AC_CHECK_LIB([sun],[main],[FCLIB="$FCLIB -lsun"],[],[])
	AC_CHECK_LIB([F77],[main],[FCLIB="$FCLIB -lF77"],[],[])
	AC_CHECK_LIB([U77],[main],[FCLIB="$FCLIB -lU77"],[],[])
	AC_CHECK_LIB([I77],[main],[FCLIB="$FCLIB -lI77"],[],[])
	AC_CHECK_LIB([isam],[main],[FCLIB="$FCLIB -lisam"],[],[])
        DEFS="$SaveDEFS"
	LIBS="$SaveLIBS"
    ;;
    alpha)
	for file in libfor.a libutil.a libFutil.a libots.a ; do
	    if test -s /usr/lib/$file ; then
		FCLIB="$FCLIB /usr/lib/$file"
            fi
	done
    ;;
    freebsd|linux)
	AC_MSG_RESULT()
        SaveDEFS="$DEFS"
        SaveLIBS="$LIBS"
	AC_CHECK_LIB([f2c],[main],[FCLIB="$FCLIB -lf2c"],[],[])
	AC_CHECK_LIB([m],[main],[FCLIB="$FCLIB -lm"],[],[])
        DEFS="$SaveDEFS"
	LIBS="$SaveLIBS"
    ;;
    hpux)
	for file in libf.a libf.sl ; do
	    if test -s /usr/lib/$file ; then
		FCLIB="$FCLIB /usr/lib/$file"
            fi
	done
    ;;
    paragon)
	AC_MSG_RESULT()
        SaveDEFS="$DEFS"
        SaveLIBS="$LIBS"
	AC_CHECK_LIB([f],[main],[FCLIB="$FCLIB -lf"],[],[])
        DEFS="$SaveDEFS"
	LIBS="$SaveLIBS"
    ;;
    *)
    :
    ;;
esac
if test -n "$FCLIB" ; then
    AC_MSG_RESULT(found $FCLIB)
else
    AC_MSG_RESULT(none)
fi
fi
])dnl
dnl Fortran extensions
dnl
dnl PAC_FORTRAN_HAS_POINTER(action-if-true,action-if-false)
dnl
dnl Checks that you can do
dnl
dnl integer M
dnl pointer (MPTR,M)
dnl data MPTR/0/
dnl 
dnl if F77_VERBOSE defined, prints why it failed to find
dnl pointer
dnl
AC_DEFUN([PAC_FORTRAN_HAS_POINTER],
[
AC_MSG_CHECKING(Fortran has pointer declaration)
cat > conftest.f <<EOF
        program main
        integer M
        pointer (MPTR,M)
        data MPTR/0/
        end
EOF
/bin/rm -f conftest.out
$F77 $FFLAGS -c conftest.f > conftest.out 2>&1
if test $? != 0 ; then
    AC_MSG_RESULT(no)
    if test -n "$F77_VERBOSE" ; then
        echo "Fortran compiler returned non-zero return code"
        if test -s conftest.out ; then
	    echo "Output from test was"
            cat conftest.out
        fi
    fi
    ifelse([$2],,:,[$2])
elif test ! -s conftest.o ; then
    AC_MSG_RESULT(no)
    if test -n "$F77_VERBOSE" ; then
        echo "Fortran compiler did not produce object file"
        if test -s conftest.out ; then
	    echo "Output from test was"
            cat conftest.out
        fi
    fi
    ifelse([$2],,:,[$2])
else    
    AC_MSG_RESULT(yes)
    ifelse([$1],,:,[$1])
fi
rm -f conftest* 
])dnl
dnl
dnl There is a bug in AC_PREPARE that sets the srcdir incorrectly (it
dnl is correct in configure, but it puts an absolute path into config.status,
dnl which is a big problem for scripts like mpireconfig that are wrappers
dnl around config.status).  The bug is in not recognizing that ./ and .//
dnl are the same  directory as . (in fact, ./[/]* is the same).
dnl
AC_DEFUN([PAC_FIXUP_SRCDIR],
[
# Find the source files, if location was not specified.
if test "$srcdirdefaulted" = "yes" ; then
  srcdir=""
  # Try the directory containing this script, then `..'.
  prog=[$]0
changequote(,)dnl
  confdir=`echo $prog|sed 's%/[^/][^/]*$%%'`
  # Remove all trailing /'s 
  confdir=`echo $confdir|sed 's%[/*]$%%'`
changequote([,])dnl
  test "X$confdir" = "X$prog" && confdir=.
  srcdir=$confdir
  if test ! -r $srcdir/$unique_file; then
    srcdir=..
  fi
fi
if test ! -r $srcdir/$unique_file; then
  if test x$srcdirdefaulted = xyes; then
    echo "configure: Cannot find sources in \`${confdir}' or \`..'." 1>&2
  else
    echo "configure: Cannot find sources in \`${srcdir}'." 1>&2
  fi
  exit 1
fi
# Preserve a srcdir of `.' to avoid automounter screwups with pwd.
# (and preserve ./ and .//)
# But we can't avoid them for `..', to make subdirectories work.
case $srcdir in
  .|./|.//|/*|~*) ;;
  *) srcdir=`cd $srcdir; pwd` ;; # Make relative path absolute.
esac
])
dnl
dnl Solaris blew the declarations for gettimeofday...
dnl
dnl PAC_IS_GETTIMEOFDAY_OK(ok_action,failure_action)
dnl
AC_DEFUN([PAC_IS_GETTIMEOFDAY_OK],
[
AC_MSG_CHECKING(for how many arguments gettimeofday takes)
# Test sets "wierd" only for FAILURE to accept 2
AC_RUN_IFELSE(AC_LANG_PROGRAM([[#include <sys/time.h>]],
[[struct timeval tp;
gettimeofday(&tp,(void*)0);return 0;]]),AC_MSG_RESULT(two - whew)
$1,
AC_MSG_RESULT(one!)
$2,AC_MSG_RESULT(asuming two)
$1)
])
dnl
dnl Set USE_STDARG if stdargs work correctly.  Sets var to 1 if it is, 
dnl 0 otherwise
dnl
dnl PAC_STDARG_CORRECT(var)
AC_DEFUN([PAC_STDARG_CORRECT],[
PAC_COMPILE_CHECK_FUNC(stdarg is correct,[
#include <stdio.h>
#include <stdarg.h>
int func( int a, ... ){
int b;
va_list ap;
va_start( ap, a );
b = va_arg(ap, int);
printf( "%d-%d\n", a, b );
va_end(ap);
fflush(stdout);
return 0;
}
int main() { func( 1, 2 ); return 0;}],$1=1,$1=0)])
dnl
dnl From aclocal.m4 in MPITEST (configure for Intel Test Suite)
dnl Added INTEGER*8
dnl
dnl PAC_TEST_FORTTYPES tests to see if the following fortran datatypes are
dnl supported: INTEGER1, INTEGER2, INTEGER4, REAL4, REAL8, DOUBLE_COMPLEX
dnl
AC_DEFUN([PAC_TEST_FORTTYPES],
   [
FIX_FILE=0
FORT_INT1=1
FORT_INT2=1
FORT_INT4=1
FORT_INT8=1
FORT_REAL4=1
FORT_REAL8=1
FORT_DOUBLE_COMPLEX=1
COUNT=13
AC_MSG_CHECKING(for integer * 1)
cat > testfort.f <<EOF
        subroutine forttype( a )
        integer*1 a
        return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_INT1=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for integer * 2)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        integer*2 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_INT2=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for integer * 4)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        integer*4 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_INT4=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for integer * 8)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        integer*8 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >>config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_INT8=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for integer * 16)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        integer*16 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_INT16=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for real * 4)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        real*4 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_REAL4=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for real * 8)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        real*8 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_REAL8=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for real * 16)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        real*16 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_REAL16=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for double complex)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        double complex a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_DOUBLE_COMPLEX=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for complex * 8)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        complex*8 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_COMPLEX8=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for complex * 16)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        complex*16 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_COMPLEX16=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
AC_MSG_CHECKING(for complex * 32)
    cat > testfort.f <<EOF
        subroutine forttype( a )
        complex*32 a
	return
        end
EOF
   echo "$F77 $FFLAGS -c testfort.f" >>config.log
   $F77 $FFLAGS -c testfort.f >> config.log 2>&1
   if test ! -s testfort.o ; then
       AC_MSG_RESULT(no)
       FORT_COMPLEX32=0
       FIX_FILE=1
       COUNT=`expr ${COUNT} - 1`
   else
       AC_MSG_RESULT(yes)
   fi
   /bin/rm -f testfort.f testfort.o
dnl
   ])dnl
dnl
dnl
dnl PAC_CHECK_COMPILER_OPTION(optionname,action-if-ok,action-if-fail)
dnl This is now careful to check that the output is different, since 
dnl some compilers are noisy.
dnl 
dnl We are extra careful to prototype the functions incase compiler options
dnl that complain about poor code are in effect
dnl
dnl Side effect: If compiler option works, it is added to CFLAGS
dnl
AC_DEFUN([PAC_CHECK_COMPILER_OPTION],[
AC_MSG_CHECKING([that C compiler accepts option $1])
CFLAGSSAV="$CFLAGS"
CFLAGS="$1 $CFLAGS"
rm -f conftest.out
echo 'int try(void);int try(void){return 0;}' > conftest2.c
echo 'int main(void);int main(void){return 0;}' > conftest.c
if ${CC-cc} $CFLAGSSAV -o conftest conftest.c >conftest.bas 2>&1 ; then
   if ${CC-cc} $CFLAGS -o conftest conftest.c >conftest.out 2>&1 ; then
      if diff -b conftest.out conftest.bas >/dev/null 2>&1 ; then
         AC_MSG_RESULT(yes)
         AC_MSG_CHECKING([that routines compiled with $1 can be linked with ones compiled  without $1])       
         /bin/rm -f conftest.out
         /bin/rm -f conftest.bas
         if ${CC-cc} -c $CFLAGSSAV conftest2.c >conftest2.out 2>&1 ; then
            if ${CC-cc} $CFLAGS -o conftest conftest2.o conftest.c >conftest.bas 2>&1 ; then
               if ${CC-cc} $CFLAGS -o conftest conftest2.o conftest.c >conftest.out 2>&1 ; then
                  if diff -b conftest.out conftest.bas >/dev/null 2>&1 ; then
	             AC_MSG_RESULT(yes)	  
                     $2
                  elif test -s conftest.out ; then
	             cat conftest.out >> config.log
	             AC_MSG_RESULT(no)
                     AC_MSG_WARN(Will not add $1 to CFLAGS.)
                     CFLAGS="$CFLAGSSAV"
	             $3
                  else
                     AC_MSG_RESULT(no)
                     AC_MSG_WARN(Will not add $1 to CFLAGS.)
                     CFLAGS="$CFLAGSSAV"
	             $3
                  fi  
               else
	          if test -s conftest.out ; then
	             cat conftest.out >> config.log
	          fi
                  AC_MSG_RESULT(no)
                  AC_MSG_WARN(Will not add $1 to CFLAGS.)
                  CFLAGS="$CFLAGSSAV"
                  $3
               fi
	    else
               # Could not link with the option!
               AC_MSG_RESULT(no)
            fi
         else
            if test -s conftest2.out ; then
               cat conftest.out >> config.log
            fi
	    AC_MSG_RESULT(no)
            AC_MSG_WARN(Will not add $1 to CFLAGS.)
            CFLAGS="$CFLAGSSAV"
	    $3
         fi
      else
         cat conftest.out >> config.log
         AC_MSG_RESULT(no)
         $3
         CFLAGS="$CFLAGSSAV"
      fi
   else
      AC_MSG_RESULT(no)
      $3
      if test -s conftest.out ; then cat conftest.out >> config.log ; fi
      CFLAGS="$CFLAGSSAV"
   fi
else
    # Could not compile without the option!
    AC_MSG_RESULT(no)
fi
rm -f conftest*
])
dnl
dnl
dnl PAC_CHECK_CPP_COMPILER_OPTION(optionname,action-if-ok,action-if-fail)
dnl This is now careful to check that the output is different, since 
dnl some compilers are noisy.
dnl 
dnl We are extra careful to prototype the functions incase compiler options
dnl that complain about poor code are in effect
dnl
dnl Side effect: If compiler option works, it is added to CXXFLAGS
dnl
AC_DEFUN([PAC_CHECK_CPP_COMPILER_OPTION],[
AC_MSG_CHECKING([that C++ compiler accepts option $1])
CXXFLAGSSAV="$CXXFLAGS"
CXXFLAGS="$1 $CXXFLAGS"
rm -f conftest.out
echo 'int cpptry(void);int cpptry(void){return 0;}' > conftest2.cpp
echo 'int main(void);int main(void){return 0;}' > conftest.cpp
if ${CXX-gcc} $CXXFLAGSSAV -o conftest conftest.cpp >conftest.bas 2>&1 ; then
   if ${CXX-gcc} $CXXFLAGS -o conftest conftest.cpp >conftest.out 2>&1 && \
      diff -b conftest.out conftest.bas >/dev/null 2>&1 ; then
      AC_MSG_RESULT(yes)
      AC_MSG_CHECKING([that routines compiled with $1 can be linked with ones compiled  without $1])
      rm -f conftest.out conftest.bas
      if ${CXX-gcc} -c $CXXFLAGSSAV conftest2.cpp >conftest2.out 2>&1 && \
         ${CXX-gcc} $CXXFLAGS -o conftest conftest2.o conftest.cpp >conftest.out 2>&1 ; then
         AC_MSG_RESULT(yes)
	 $2
      else
	 if test -s conftest.out ; then cat conftest.out >> config.log; fi
	 AC_MSG_RESULT(no)
	 AC_MSG_WARN(Will not add $1 to CXXFLAGS.)
	 CXXFLAGS="$CXXFLAGSSAV"
	 $3
      fi
   else
      if test -s conftest.out ; then cat conftest.out >> config.log; fi
      AC_MSG_RESULT(no)
      CXXFLAGS="$CXXFLAGSSAV"
      $3
   fi
else
    # Could not compile without the option!
    AC_MSG_RESULT(no)
fi
rm -f conftest*
])
dnl
dnl PAC_CHECK_FC_COMPILER_OPTION is like PAC_CHECK_COMPILER_OPTION,
dnl except for Fortran 
dnl It is harder to do a test here since Fortran compilers tend to be very 
dnl noisy.
dnl
dnl Side effect: If compiler option works, it is added to FFLAGS
dnl
AC_DEFUN([PAC_CHECK_FC_COMPILER_OPTION],[
AC_MSG_CHECKING([that Fortran compiler accepts option $1])
FFLAGSSAV="$FFLAGS"
FFLAGS="$1 $FFLAGS"
cat >conftest.f <<EOF
        program main
        end
EOF
cat >conftest3.f <<EOF
       subroutine try( )
       return
       end
EOF
/bin/rm -f conftest1.out conftest2.out
/bin/rm -f conftest3.out
if $F77 $FFLAGS -o conftest conftest.f > conftest1.out 2>&1 ; then
    if $F77 $FFLAGSSAV -o conftest conftest.f > conftest2.out 2>&1 ; then
        if diff conftest2.out conftest1.out > /dev/null 2>&1 ; then
            AC_MSG_RESULT(yes)
            AC_MSG_CHECKING([that routines compiled with $1 can be linked with ones compiled without $1])
            /bin/rm -f conftest1.out 
            if $F77 -c $FFLAGSSAVE conftest3.f >conftest3.out 2>&1 ;then
                if $F77 $FFLAGS -o conftest conftest3.o conftest.f >conftest1.out 2>&1 ; then
                    if diff conftest2.out conftest1.out > /dev/null 2>&1 ; then
                        AC_MSG_RESULT(yes)  
			$2
	            else
                        AC_MSG_RESULT(no)
			FFLAGS="$FFLAGSSAV"
                        cat conftest1.out >> config.log
                        $3
	            fi
                else
                    AC_MSG_RESULT(no)
		    FFLAGS="$FFLAGSSAV"
                    cat conftest1.out >> config.log
                    $3
                fi
            else
                 AC_MSG_RESULT(no)
	         FFLAGS="$FFLAGSSAV"
                 cat conftest3.out >> config.log
                 $3
            fi
       else
           AC_MSG_RESULT(no)
	   FFLAGS="$FFLAGSSAV"
	   $3
       fi
    else
        AC_MSG_RESULT(no)
        FFLAGS="$FFLAGSSAV"
	$3
    fi
else
    AC_MSG_RESULT(no)
    FFLAGS="$FFLAGSSAV"
    $3
fi
rm -f conftest*

])
dnl
dnl
dnl Check that shared library support actually works
dnl
dnl Extra symbols used:
dnl    CC_SHARED_OPT
dnl    SHARED_LIB_UTIL (mpich/util/makesharedlib -kind=@SHAREDKIND@ -local)
dnl    SHARED_LIB_PATH (how to get linker to look in current directory
dnl                     for a shared library).  
dnl    SHARED_LIB_SEARCH_PATH (How to specify that the PROGRAM should look
dnl                     in a particular directory for a shared library.
dnl                     It should include `pwd`
dnl
dnl    Also checks to see if a program remembers where the shared library
dnl    was (!).  Some systems require LD_LIBRARY_PATH be set(!)
dnl    Set SHARED_LIB_NEEDS_PATH to yes if LD_LIBRARY_PATH needed
dnl PAC_SHARED_LIBS_OK([action-if-ok],[action-if-failed])
dnl
AC_DEFUN([PAC_SHARED_LIBS_OK],
[
    if test -z "$SHARED_LIB_UTIL" ; then
	echo "Error in configure; SHARED_LIB_UTIL not set"
	exit 1
    fi
    AC_MSG_CHECKING([that shared libraries can be built])
    rm -f conftest* libconftest*
    cat >conftest.c <<EOF
int foo(int a);
int foo(int a) {return a+1;}
EOF
    cat >conftest1.c <<EOF
int main(void);
int foo(int);
int main(void){ return foo(-1); }
EOF
    if ${CC-cc} $CFLAGS $CC_SHARED_OPT -c conftest.c >conftest.out 2>&1 ; then
	$SHARED_LIB_UTIL -obj=conftest.o -lib=libconftest.a
	if ${CC-cc} $CFLAGS -o conftest conftest1.c $SHARED_LIB_PATH \
		$SHARED_LIB_SEARCH_PATH -lconftest >conftest.out 2>&1 ; then
	   ifelse([$1],,,[$1])
	   AC_MSG_RESULT(yes)
	   AC_MSG_CHECKING(that programs remember where the shared lib is)
           mkdir .tmp
           cp conftest .tmp
	   cd .tmp 
	      if ./conftest >conftest.out 2>&1 ; then
		  AC_MSG_RESULT(yes)
	      else
		  if test -s conftest.out ; then 
			cat conftest.out >>../config.log
		  fi
		  # Try with LD_LIBRARY_PATH
		  saveLD="$LD_LIBRARY_PATH"
		  if test -z "$LD_LIBRARY_PATH" ; then LD_LIBRARY_PATH="." ; fi
	          LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:`pwd`/.."
		  export LD_LIBRARY_PATH
		  if ./conftest >>conftest.out 2>&1 ; then
		      AC_MSG_RESULT(no: needs path in LD_LIBRARY_PATH!)
		      SHARED_LIB_NEEDS_PATH="yes"
		  else
		      AC_MSG_RESULT(no: LD_LIBRARY_PATH does not work!)
		  fi
		  LD_LIBRARY_PATH="$saveLD"
	      fi
           cd ..
	   rm -rf .tmp
	else
           ifelse([$2],,,[$2])
	   if test -s conftest.out ; then cat conftest.out >> config.log ; fi
	   AC_MSG_RESULT(no)
	fi
    else
        ifelse([$2],,,[$2])
	if test -s conftest.out ; then cat conftest.out >> config.log ; fi
	AC_MSG_RESULT(no)
    fi
rm -f conftest* libconftest*
])
dnl
dnl Test that the C compiler allows #define a(b) a(b,__LINE__,__FILE__)
dnl PAC_MACRO_NAME_IN_MACRO([action if ok],[action if failed])
dnl
dnl Note that we can't put a pound sign into the msg_checking macro because
dnl it confuses autoconf
AC_DEFUN([PAC_MACRO_NAME_IN_MACRO],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_MSG_CHECKING([that compiler allows define a(b) a(b,__LINE__)])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
void a(i,j)int i,j;{}
#define a(b) a(b,__LINE__)]],
[[a(0);return 0;]])],[ac_cv_cpprworks="yes"],[ac_cv_cpprworks="no"])
if test $ac_cv_cpprworks = "yes" ; then
    AC_MSG_RESULT(yes)
    ifelse([$1],,,[$1])
else
    AC_MSG_RESULT(no)
    ifelse([$2],,,[$2])
fi
])
dnl
dnl Check that anonymous mmap works
dnl
dnl PAC_HAVE_ANON_MMAP([action-if-success],[action-if-failure])
AC_DEFUN([PAC_HAVE_ANON_MMAP],[
AC_CHECK_FUNCS(mmap)
# Check that MMAP works!
AC_MSG_CHECKING([that shared, anonymous mmap works with -1 filedes])
AC_RUN_IFELSE(AC_LANG_PROGRAM([[
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
]],[[
int memsize;
caddr_t shmptr;
memsize = getpagesize();
shmptr = mmap((caddr_t) 0, memsize, 
PROT_READ|PROT_WRITE|PROT_EXEC, 
MAP_SHARED
#ifdef MAP_ANON
|MAP_ANON
#endif
,-1, (off_t) 0);
if (shmptr == (caddr_t) -1) {
 return 1;}
]]),ac_cv_mmap=1,ac_cv_mmap=0,ac_cv_mmap=1)
if test $ac_cv_mmap = 0 ; then 
    AC_MSG_RESULT(no!)
    ifelse($1,,,$1)
else
    AC_MSG_RESULT(yes)
    ifelse($2,,,$2)
fi
])
dnl
dnl Check that semget works correctly (sometimes no enabled, sometimes only
dnl root can use it).  
dnl
dnl PAC_SEMGET_WORKS([action-if-success],[action-if-failure])
AC_DEFUN([PAC_SEMGET_WORKS],[
# We need to check that semctl is ok.
# Both of these need to go into aclocal.m4
AC_MSG_CHECKING([that semget works])
###
### Still need to check for SEMUN_UNDEFINED - see mpid/ch_p4/p4/configure.in
### 
cat > conftest.c <<EOF
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/sem.h>
int main () {
#ifdef NEEDS_UNION_SEMUN
#if defined(SEMUN_UNDEFINED)    
union semun { int val } arg;
#else
union semun arg;
arg.val = 0;
#endif
#else
int arg = 0;
#endif
key_t key;
int semset_id;
key=ftok(".", 'a');
errno=0;
if ((semset_id=semget(key,10,IPC_CREAT|IPC_EXCL|0666)) == -1) 
printf("%d\n", errno);
else {
printf("%d\n", errno);
semctl(semset_id,0,IPC_RMID,arg); }
return 0; 
}
EOF
echo "${CC-cc} $CFLAGS -o conftest conftest.c $LIBS" >> config.log
if ${CC-cc} $CFLAGS -o conftest conftest.c $LIBS >> config.log 2>&1 ; then
    if test -x conftest ; then
	/bin/rm -f conftest.out
	./conftest > conftest.out
	errno=`cat conftest.out`

	# these values are specific to a particular unix.
        # we need to convert number to ERRNO based on the local 
        # system, and then match, if we can
	if test $errno -eq 0 ; then
	    AC_MSG_RESULT(yes)
	    ifelse($1,,,$1)
	elif test $errno -eq 13 ; then
	    AC_MSG_RESULT(no)
	    ifelse($2,,,$2)
	    AC_MSG_WARN([No access permission rights to the semaphore set
		created with this key! Configure could have chosen to create a
		semaphore set using an unacceptable key value])
        elif test $errno -eq 17 ; then
	    AC_MSG_RESULT(no)
	    ifelse($2,,,$2)
	    AC_MSG_WARN([The semaphore set created with this key already
		exists! Try running util/cleanipcs and then reconfiguring. This
		may or may not help.])
	elif test $errno -eq 43 ; then
	    AC_MSG_RESULT(no)
	    ifelse($2,,,$2)
	    AC_MSG_WARN([The semaphore set created with this key is marked to
		be deleted! Try running util/cleanipcs and then reconfiguring.
		This may or may not help.])
	elif test $errno -eq 2 ; then
	    AC_MSG_RESULT(no)
	    ifelse($2,,,$2)
	    AC_MSG_WARN([No semaphore set exists for this key! Configure could
		have chosen to create a semaphore set using an unacceptable key
		value.])
	elif test $errno -eq 12 ; then
	    AC_MSG_RESULT(undetermined)
	    ifelse($2,,,$2)
	    AC_MSG_WARN([Not enough memory to create a semaphore set! Try
		running util/cleanipcs and then reconfiguring. This may or may
		not help.])
	elif test $errno -eq 28 ; then
	    AC_MSG_RESULT(no)
	    ifelse($2,,,$2)
	    AC_MSG_WARN([The system limit for the maximum number of semaphore
		sets (SEMMNI), or the system wide maximum number of semaphores
		(SEMMNS) has been reached.])
        fi
    else
	AC_MSG_RESULT(undetermined)
        ifelse($2,,,$2)
	AC_MSG_WARN([Could not build executable])
    fi
else
    AC_MSG_RESULT(undetermined)
    ifelse($2,,,$2)
    AC_MSG_WARN([Could not compile program])
fi
/bin/rm -f conftest*
])
dnl
dnl
dnl The following trys a program and sets a flag to indicate whether
dnl the compilation is clean (no extra messages), successful (messages but
dnl status is zero), or failed.  This handles the case where the compiler
dnl complains about a type mismatch, but allows it anyway, and it can be
dnl used to find the appropriate type.
dnl
dnl PAC_TRY_COMPILE_CLEAN(includes,function,flagvar)
dnl flagvar is set to 0 (clean), 1 (dirty but status ok), 2 (failed)
dnl Note that an entire function is needed as the second argument,
dnl not just a function body.  This allows us to check for more 
dnl complex situations (such as handling ... in an arg list)
dnl
AC_DEFUN([PAC_TRY_COMPILE_CLEAN],[
$3=2
rm -f conftest*
dnl 
dnl Get the compiler output to test against
if test -z "$TRY_COMPLILE_CLEAN" ; then
    echo 'int try(void);int try(void){return 0;}' > conftest.c
    if ${CC-cc} $CFLAGS -c conftest.c >conftest.bas 2>&1 ; then
	if test -s conftest.bas ; then 
	    TRY_COMPILE_CLEAN_OUT=`cat conftest.bas`
        fi
        TRY_COMPILE_CLEAN=1
    else
	AC_MSG_WARN([Could not compile simple test program!])
	if test -s conftest.bas ; then 	cat conftest.bas >> config.log ; fi
    fi
fi
dnl
dnl Create the program that we need to test with
rm -f conftest*
cat >conftest.c <<EOF
#include "confdefs.h"
[$1]
[$2]
EOF
dnl
dnl Compile it and test
if ${CC-cc} $CFLAGS -c conftest.c >conftest.bas 2>&1 ; then
    dnl Success.  Is the output the same?
    if test "$TRY_COMPILE_CLEAN_OUT" = "`cat conftest.bas`" ; then
	$3=0
    else
        cat conftest.c >>config.log
	if test -s conftest.bas ; then 	cat conftest.bas >> config.log ; fi
        $3=1
    fi
else
    dnl Failure.  Set flag to 2
    cat conftest.c >>config.log
    if test -s conftest.bas ; then cat conftest.bas >> config.log ; fi
    $3=2
fi
rm -f conftest*
])
dnl
