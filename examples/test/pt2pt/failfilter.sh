#!/bin/bash

# define if success output should be shown (defaulkt is off)
SHOWSUCC=off

# file extions for files thad define positive matching criteria
SUCCEXT="std"

# file extions for files thad define negative matching criteria
FAILEXT="failure"

# this file indicates output wich should be ignored
IGNOREFILE="ignore.std"

# default test has completely passed phrase
DEFAULTPOS="defaultpass.std"

NOOUTPUT=""

# filtering cmdlineopts
case $1 in
  -list)
     NOOUTPUT=1
     shift
     ;;
  -showall)
     SHOWSUCC=on
     shift
     ;;
esac



if test -n "$1" ; then
    FILES="$1.out"
else
    FILES=`ls *.out`
fi


for i in $FILES ; do
    BASENAME=`echo $i | sed 's/\.out//'`
    FROMFILE=""
    SUCCESS=""

    # echo $BASENAME.$SUCCEXT

    # tests for positives
    # a positive is only satisfied if all of the defined lines occur
    if test -f $BASENAME.$SUCCEXT ; then
        FROMFILE="1"

	# echo "$BASENAME positive match"

	# RESULT=`egrep -f $BASENAME.$SUCCEXT $BASENAME.out | sort | uniq`
	cat $BASENAME.$SUCCEXT | sed 's/^$/____loeschen____/' | egrep -v ____loeschen____  >regex.tmp
	WC1=`egrep -f regex.tmp $i | sort | uniq |wc -l`
	WC2=`sort regex.tmp | uniq | wc -l`
	rm regex.tmp

	# echo "WC1 $WC1"
	# echo "WC2 $WC2"

	if test $WC1 -lt $WC2 ; then
	    if test -n "$NOOUTPUT" ; then
		echo $BASENAME
	    else
		echo
		echo "----- $BASENAME has failed (pos.) - output: -----"
		egrep -v -f $IGNOREFILE $BASENAME.out
		echo
		echo
	    fi
	SUCCESS=""
	else
	    SUCCESS="1"
        fi
    fi

    # tests for negatives
    # a negatiove test is satisfied if any of the defined lines occures
    if test -f $BASENAME.$FAILEXT ; then
        FROMFILE="1"

	# echo "$BASENAME negative match"

	RESULT=`egrep -f $BASENAME.$FAILEXT $BASENAME.out`
	if test -n "$RESULT" ; then
	    if test -n "$NOOUTPUT" ; then
		echo $BASENAME
	    else
		echo
		echo "----- $BASENAME has failed (neg.) - output: -----"
		egrep -v -f $IGNOREFILE $BASENAME.out
		echo
		echo
	    fi
	    SUCCESS=""
	else
	    SUCCESS="1"
        fi
    fi

    #default criteria if none is given
    if test -z "$FROMFILE" ; then
	if test -f $DEFAULTPOS ; then
	    RESULT=`egrep -f $DEFAULTPOS $BASENAME.out`
	    if test -z "$RESULT" ; then
		if test -n "$NOOUTPUT" ; then
		    echo $BASENAME
		else
		    echo
		    echo "----- $BASENAME has failed (default) - output: -----"
		    egrep -v -f $IGNOREFILE $BASENAME.out
		    echo
		    echo
		fi
	    SUCCESS=""
	    else
	        SUCCESS="1"
	    fi
	else
	    echo "### warning, no default passfile!" 
	fi
    fi

    if test "$SHOWSUCC" = "on" -a -n "$SUCCESS" ; then
	if test -n "$NOOUTPUT" ; then
	    echo $BASENAME
	else
  	    echo
            echo "----- $BASENAME was successfully completed - output: -----"
            egrep -v -f $IGNOREFILE $BASENAME.out
            echo
            echo
	fi
    fi
done
