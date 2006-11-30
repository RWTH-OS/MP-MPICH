#! /bin/sh
#
# $Id$
#
#set -x

ROOT_DIR=`pwd`

# for the release version, move the relevant files
if [ $# -gt "0" ] ; then 
    if [ $1 = "nocheck" ] ; then
	diffiles=""
    fi
else
    echo "testing CVS integrity"
    # test if there are CVS files in the current directory which are not up-to-date
    diffiles=`cvs status 2> /dev/null | grep File | awk '$4 != "Up-to-date" { print $2 }'`
fi

if [ ! -n "$diffiles" ] ; then
    mkdir __distribution
    cd __distribution
    TAR_DIR=`pwd`

    # checkout ch_smi in ./mpid/ch_smi
    echo "checking out ch_smi etc."
    cvs checkout mp-mpich/mpid/ch_smi 2>/dev/null >/dev/null
    cvs checkout mp-mpich/mpid/lfbs_common 2>/dev/null >/dev/null
    cvs checkout mp-mpich/mpid/ch2/dev.h 2>/dev/null >/dev/null
    cvs checkout mp-mpich/util/ 2>/dev/null >/dev/null
  
    cd mp-mpich
    MPICH_DIR=`pwd`
    cd mpid/ch_smi
    CH_SMI_DIR=`pwd`

    # remove CVS directories
    echo "removing CVS directories and original files"
    find . -type d -name CVS -exec rm -r {} \;
    find . -name \*.orig -exec rm {} \;

    echo "fixing files"
    for dir in "install/1.1.2" "install/1.2.0" ; do
	cd $dir

	# remove files which should not become tar'ed
	rm Makefile.in configure util/mpirun.*in

	# add mpirun.ch_smi.in to the archive
	cp ${MPICH_DIR}/util/mpirun.ch_smi.in ./util

	# for the 1.2.0, get some files from the MP-MPICH source tree
	if [ $dir = "install/1.2.0" ] ; then
	    cp ${MPICH_DIR}/mpid/ch2/dev.h ./mpid/ch_smi
	    cp ${MPICH_DIR}/mpid/lfbs_common/adi2cancel.c ./mpid/ch_smi
	    mv ${CH_SMI_DIR}/Makefile.in ./mpid/ch_smi
	fi

	cd $CH_SMI_DIR
    done

    # make links for installation script
    cd ${TAR_DIR}/mp-mpich
    ln -s mpid/ch_smi/install.sh ./install-ch_smi
    
    # put everything together
    echo "tar'ing & zipping"
    tar cf ${ROOT_DIR}/ch_smi.tar mpid/ch_smi ./install-ch_smi

    # clean up
    cd ${ROOT_DIR}
    gzip ch_smi.tar

    echo "cleaning up"
    rm -r __distribution
    echo "ch_smi.tar.gz now contains the current state of ch_smi."
    echo "Please check that patches are valid!"
else
    echo "The following files are not Up-to-date:"
    echo "$diffiles"
fi
