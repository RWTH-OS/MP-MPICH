#! /bin/sh
# $Id: install.sh,v 1.5 2000/05/15 12:25:19 joachim Exp $

# this scripts install the necessary files which reside outside from mpid/ch_smi
#

#set -x


# static presets
LINUX_PATCH_CMD="patch -b .orig"
SUNOS_PATCH_CMD="patch -b -i"
INSTALL_FILES="uninstall-ch_smi configure Makefile.in util/mpirun.args.in \
               util/mpirun.ch_smi.in mpid/ch_smi/dev.h mpid/ch_smi/Makefile.in"

# dynamic presets
PWD=`pwd`
HOST_OS=`uname`
# for the release version, move the relevant files
if [ $# -gt "0" ] ; then 
  if [ $1 = "release" ] ; then
    LINK="mv -f"
  fi
else
  LINK="ln -s"
fi

# look where we are and set pathes
if [ -d "mpid/ch_smi" ] ; then
    SRC_DIR=${PWD}/mpid/ch_smi/install
    DST_DIR="."
elif [ -r $PWD/smipriv.c ] ; then
    SRC_DIR=${PWD}/install
    DST_DIR="../.."
else
    echo "*** Error: Could not find the ch_smi device directory. "
    echo "This scripts must be executed from the MPICH root directory or"
    echo "from within the mpid/ch_smi directory."
    echo ""
    echo "Installation aborted."
    exit 1
fi

# patch a file with backup file creation
patch_file()
{
    PATCH=$1
    DEST=$2

    case $HOST_OS in
    Linux)
	$LINUX_PATCH_CMD $DEST $PATCH
	;;
    SunOS)
	$SUNOS_PATCH_CMD $PATCH $DEST
	;;
    *)
	echo "*** Error: Unknown Operating System"
	echo "Supported are Linux and SunOS"
	echo "Check output from uname"
	exit 1
	;;
    esac

    return
}


# install a file by linking or patching
install_file ()
{
    DESTFILE=$1

    if [ -r ${SRC_DIR}/${DESTFILE} ] ; then
	$LINK ${SRC_DIR}/${DESTFILE} ${DST_DIR}/${DESTFILE}
    elif [ -r ${SRC_DIR}/${DESTFILE}.patch ] ; then
	patch_file ${SRC_DIR}/${DESTFILE}.patch ${DST_DIR}/${DESTFILE}
    else
	echo "*** Error: can't install "${DESTFILE}
    fi

    return
}

# is SCI-MPICH not yet installed ?
if [ -r ${DST_DIR}/util/mpirun.ch_smi.in ] ; then
    echo "*** Error: ch_smi is already installed."
    echo ""
    echo "Installation aborted."
    exit 1
fi
    
# check which MPICH version is installed (supported are 1.1.2 and 1.2.0)  
README_FILENAME="${DST_DIR}/README"
MPICH_VERSION=`cat $README_FILENAME | gawk 'FNR == 4 { print $2 }'`
if [ "$MPICH_VERSION" = "1.1.2," ] ; then
    MPICH_VERSION="1.1.2"
    INSTALL_FILES="${INSTALL_FILES} util/mpirun.sh.in"
elif [ "$MPICH_VERSION" = "1.2.0," ] ; then
    MPICH_VERSION="1.2.0"
    INSTALL_FILES="${INSTALL_FILES} util/mpirun.in mpid/ch_smi/adi2cancel.c"
else
    echo "*** Error: Could not determine MPICH version."
    echo "Supported versions are 1.1.2 and 1.2.0."
    echo "Appropriate README in MPICH root directory needed."
    echo ""
    echo "Installation aborted."
    exit 1
fi

# set source path for installation
SRC_DIR=${SRC_DIR}/${MPICH_VERSION}

echo "Installing ch_smi device into the current MPICH $MPICH_VERSION distribution"
echo "The original MPICH files will be backed up with .orig suffix"
echo "You can revert to the original MPICH distribution using uninstall-ch_smi"

# test if everything is where it should be
abort=0
if [ ! -w "${DST_DIR}" ] ; then
    echo "No write access to MPICH directory!"
    abort=1
elif [ ! -w ${DST_DIR}/configure ] ; then
    echo "MPICH configure script not found!"
    abort=1
elif [ ! -w ${DST_DIR}/Makefile.in ] ; then
    echo "MPICH root Makefile.in not found!"
    abort=1
elif [ ! "(" -d ${DST_DIR}/util -a -w ${DST_DIR}/util ")" ] ; then
    echo "MPICH util directory not found!"
    abort=1
fi
if [ $abort = 1 ] ; then
    echo "Installation aborted."
    exit 1
fi

# o.k., everything is fine, here we go
for inst_file in $INSTALL_FILES ; do
    install_file $inst_file
done

echo ""
echo "ch_smi installation complete. Now launch 'configure' in the MPICH root"
echo "directory with the required options (see SCI-MPICH documentation)"

exit 0
