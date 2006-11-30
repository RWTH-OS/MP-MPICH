#! /bin/sh
# $Id$
#set -x

echo "testing patch-file consistency"
# test if the patch files are up-to-date
patches=`find . -name \*.orig`
for origfile in $patches ; do
    filename=`echo $origfile | sed s/.orig//g`
    patchfile=${filename}.patch
    difffile=/tmp/__`basename $filename`.tst
    diffdiff=/tmp/__`basename $filename`.diff

    diff $origfile $filename >$difffile
    diff $difffile $patchfile >$diffdiff
    if [ -s $diffdiff ] ; then
	lines=`wc -l $diffdiff | gawk '{ print $1 }'`
	idlines=`cat $diffdiff | grep Id:`
	if [ $lines -gt 1 -o -z "idlines" ] ; then
	    echo "creating new patch for $filename"
	    diff $origfile $filename >$patchfile
	else
	    echo "patch for $filename is up-to-date."
	fi   
    else
	echo "patch for $filename is up-to-date."
    fi   
    rm -f $difffile $diffdiff
done

