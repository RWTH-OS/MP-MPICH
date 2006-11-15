dnl
dnl Check for extra include dir and add it to EXTRA_CFLAGS
dnl This macro is silent, please use wi_EXTRA_DIRS
dnl
AC_DEFUN([wi_EXTRA_IDIR],[
if test -r "$1" ; then
	case $EXTRA_CFLAGS in
		*-I"$1"*)
			;;
		*)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -I$1"
			wi_result="include $wi_result"
			;;
	esac
fi
])
dnl
dnl Check for extra lib dir and add it to EXTRA_LDFLAGS
dnl This macro is silent, please use wi_EXTRA_DIRS
dnl
AC_DEFUN([wi_EXTRA_LDIR],[
if test -r "$1" ; then
	case $EXTRA_LDFLAGS in
		*-L"$1"*)
			;;
		*)
			EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$1"
			wi_result="lib $wi_result"
			;;
	esac
fi
])
dnl
dnl Check for extra program dir and add it to the PATH
dnl This macro is silent, please use wi_EXTRA_DIRS
dnl
AC_DEFUN([wi_EXTRA_PDIR],[
if test -r "$1" ; then
	case $PATH in
		*:"$1":*|"$1":*|*:"$1")
			;;
		*)
			PATH="$PATH:$1"
			wi_result="bin $wi_result"
			;;
	esac
fi
])
dnl
dnl If you want to look for a subdirectory in include/lib directories,
dnl you pass the name in argument 2.
dnl
AC_DEFUN([wi_EXTRA_DIRS],[
for exdir in $1 ; do
	AC_MSG_CHECKING([for extra include and lib directory in $exdir/*/$2])
	wi_result=""
	if test "$exdir" != "/usr" || test "$2" != ""; then
		wi_EXTRA_IDIR($exdir/include/$2)
		wi_EXTRA_LDIR($exdir/lib/$2)
		wi_EXTRA_PDIR($exdir/bin/$2)
	fi
	if test "x$wi_result" != "x"; then
		AC_MSG_RESULT($wi_result)
	else
		AC_MSG_RESULT(nothing)
	fi
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
