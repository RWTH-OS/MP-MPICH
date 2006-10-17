#!/bin/sh

# This script is only needed to regenerate configure scripts and header templates

# Some devices have their own configure scripts, but need macros from the global
# aclocal.m4, so make some symlinks

trap "echo; echo 'Error! $0 did not run correctly!'" 0
set -e

for i in . mpid/ch_shmem mpid/nt2unix mpid/ch_smi/SMI
do
	echo "Directory: $i"
	if [ -d $i/m4 ]
	then
		export ACLOCAL="aclocal -I m4"
	else
		unset ACLOCAL
	fi
	(cd $i; autoreconf -Wall)
done

echo "All done!"
trap "" 0
