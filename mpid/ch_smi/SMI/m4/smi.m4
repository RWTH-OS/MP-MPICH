dnl
dnl stolen from tclobj-1.x, which in turn stole it from ncftp-2.3.x
dnl
AC_DEFUN([wi_EXTRA_IDIR],[
incdir="$1"
if test -r $incdir ; then
	case "$CXXFLAGS" in
		*-I${incdir}*)
			# echo "   + already had $incdir" 1>&6
			;;
		*)
			if test "$CXXFLAGS" = "" ; then
				CXXFLAGS="-I$incdir"
			else
				CXXFLAGS="$CXXFLAGS -I$incdir"
			fi
			EXTRA_CXXFLAGS="-I$incdir"
			echo "   + found $incdir" 1>&6
			;;
	esac
	case "$CPPFLAGS" in
		*-I${incdir}*)	
			# echo "   + already had $incdir" 1>&6
			;;
		*)
			if test "$CPPFLAGS" = "" ; then
				CPPFLAGS="-I$incdir"
			else
				CPPFLAGS="$CPPFLAGS -I$incdir"
			fi
			#echo "   + found $incdir" 1>&6
			EXTRA_CPPFLAGS="-I$incdir"
			;;
	esac
	case "$CFLAGS" in
		*-I${incdir}*)	
			# echo "   + already had $incdir" 1>&6
			;;
		*)
			if test "$CFLAGS" = "" ; then
				CFLAGS="-I$incdir"
			else
				CFLAGS="$CFLAGS -I$incdir"
			fi
			EXTRA_CFLAGS="-I$incdir"
			#echo "   + found $incdir" 1>&6
			;;
	esac
fi
])
dnl
dnl
dnl
dnl
AC_DEFUN([wi_EXTRA_LDIR],[
local_libdir="$1"
if test -r $local_libdir ; then
	case "$LDFLAGS" in
		*-L${local_libdir}*)
			# echo "   + already had $local_libdir" 1>&6
			;;
		*)
			if test "$LDFLAGS" = "" ; then
				LDFLAGS="-L$local_libdir"
			else
				LDFLAGS="$LDFLAGS -L$local_libdir"
			fi
			EXTRA_LDFLAGS="-L$local_libdir"
			echo "   + found $local_libdir" 1>&6
			;;
	esac
fi
])
dnl
dnl __FP__
dnl
dnl
AC_DEFUN([wi_EXTRA_PDIR],[
progdir="$1"
if test -r $progdir ; then
	case "$PATH" in
		*:${progdir}*)
			# echo "   + already had $progdir" 1>&6
			;;
		*${progdir}:*)
			# echo "   + already had $progdir" 1>&6
			;;
		*)
			if test "$PATH" = "" ; then
				PATH="$progdir"
			else
				PATH="$PATH:$progdir"
			fi
			echo "   + found $progdir" 1>&6
			;;
	esac
fi
])
dnl
dnl
dnl If you want to also look for include and lib subdirectories in the
dnl $HOME tree, you supply "yes" as the first argument to this macro.
dnl
dnl If you want to look for subdirectories in include/lib directories,
dnl you pass the names in argument 3, otherwise pass a dash.
dnl
AC_DEFUN([wi_EXTRA_DIRS],[echo "checking for extra include and lib directories..." 1>&6
ifelse([$1], yes, [dnl
b1=`cd .. ; pwd`
b2=`cd ../.. ; pwd`
exdirs="$HOME $j $b1 $b2 $prefix $2"
],[dnl
exdirs="$prefix $2"
])
subexdirs="$3"
if test "$subexdirs" = "" ; then
	subexdirs="-"
fi
for subexdir in $subexdirs ; do
if test "$subexdir" = "-" ; then
	subexdir=""
else
	subexdir="/$subexdir"
fi
for exdir in $exdirs ; do
	if test "$exdir" != "/usr" || test "$subexdir" != ""; then
		incdir="${exdir}/include${subexdir}"
		wi_EXTRA_IDIR($incdir)

		local_libdir="${exdir}/lib${subexdir}"
		wi_EXTRA_LDIR($local_libdir)

		progdir="${exdir}/bin${subexdirr}"
		wi_EXTRA_PDIR($progdir)
	fi
done
done
])
dnl
dnl check if a function has a specific prototype
dnl AC_CHECK_FUNC_PROTO(FUNCTION, CODE [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([AC_CHECK_FUNC_PROTO],[
AC_MSG_CHECKING([for $1])
ac_func_var=`echo $1 | sed 'y%./+-%__p_%'`
AC_CACHE_VAL(ac_cv_func_proto_$ac_func_var,
[AC_LINK_IFELSE([AC_LANG_PROGRAM(,
 [[$2]])],
 [eval "ac_cv_func_proto_$ac_func_var=yes"],
 [eval "ac_cv_func_proto_$ac_func_var=no"])
])dnl
if eval "test \"`echo '$ac_cv_func_proto_'$ac_func_var`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], , , [$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4], , , [$4])dnl
fi
])
