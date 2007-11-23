/* 
 *   $Id: ad_fstype.c 148 2000-05-16 19:15:01Z joachim $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"
#if (defined(HPUX) || defined(SPPUX) || defined(IRIX) || defined(SOLARIS) || defined(AIX) || defined(DEC) || defined(CRAY))
#include <sys/statvfs.h>
#endif
#ifdef LINUX
#include <sys/vfs.h>
/* #include <linux/nfs_fs.h> this file is broken in newer versions of linux */
#define NFS_SUPER_MAGIC 0x6969
#endif
#ifdef FREEBSD
#include <sys/param.h>
#include <sys/mount.h>
#endif
#ifdef PARAGON
#include <nx.h>
#include <pfs/pfs.h>
#include <sys/mount.h>
#endif
#ifdef SX4
#include <sys/stat.h>
#endif
#ifdef PVFS
#include "pvfs_config.h"
#endif

void ADIO_FileSysType(char *filename, int *fstype, int *error_code)
{
    char *dir, *slash;
    int err;
#if (defined(HPUX) || defined(SPPUX) || defined(IRIX) || defined(SOLARIS) || defined(AIX) || defined(DEC) || defined(CRAY))
    struct statvfs vfsbuf;
#endif
#if (defined(LINUX) || defined(FREEBSD))
    struct statfs fsbuf;
#endif
#ifdef PARAGON
    struct estatfs ebuf;
#endif
#ifdef SX4
    struct stat sbuf;
#endif

    dir = strdup(filename);
    slash = strrchr(dir, '/');
    if (!slash) strcpy(dir, ".");
    else {
	if (slash == dir) *(dir + 1) = 0;
	else *slash = '\0';
    }

#if (defined(HPUX) || defined(SPPUX) || defined(IRIX) || defined(SOLARIS) || defined(AIX) || defined(DEC) || defined(CRAY))
    err = statvfs(filename, &vfsbuf);
    if (err && (errno == ENOENT)) err = statvfs(dir, &vfsbuf);
    free(dir);

    if (err) *error_code = MPI_ERR_UNKNOWN;
    else {
	/* FPRINTF(stderr, "%s\n", vfsbuf.f_basetype); */
	if (!strncmp(vfsbuf.f_basetype, "nfs", 3)) {
	    *fstype = ADIO_NFS;
	} else {
# if (defined(HPUX) || defined(SPPUX))
#    ifdef HFS
	    *fstype = ADIO_HFS;
#    else
            *fstype = ADIO_UFS;
#    endif
	    if (!strncmp(vfsbuf.f_basetype, "xfs", 3)) *fstype = ADIO_XFS;
	    else if (!strncmp(vfsbuf.f_basetype, "piofs", 4)) *fstype = ADIO_PIOFS;
	    else *fstype = ADIO_UFS;
# endif
	}
	*error_code = MPI_SUCCESS;
    }
#elif defined(LINUX)
    err = statfs(filename, &fsbuf);
    if (err && (errno == ENOENT)) err = statfs(dir, &fsbuf);
    free(dir);

    if (err) *error_code = MPI_ERR_UNKNOWN;
    else {
	/* FPRINTF(stderr, "%d\n", fsbuf.f_type);*/
	if (fsbuf.f_type == NFS_SUPER_MAGIC) *fstype = ADIO_NFS;
#ifdef PVFS
	else if (fsbuf.f_type == PVFS_SUPER_MAGIC) *fstype = ADIO_PVFS;
#elif defined(SVM)
	else *fstype = ADIO_SVM;
#else
	else *fstype = ADIO_UFS;
#endif
	*error_code = MPI_SUCCESS;
    }
#elif (defined(FREEBSD) && defined(HAVE_MOUNT_NFS))
    err = statfs(filename, &fsbuf);
    if (err && (errno == ENOENT)) err = statfs(dir, &fsbuf);
    free(dir);

    if (err) *error_code = MPI_ERR_UNKNOWN;
    else {
#if (__FreeBSD_version>300004)
	if ( !strncmp("nfs",fsbuf.f_fstypename,3) ) *fstype = ADIO_NFS;
#else
	if (fsbuf.f_type == MOUNT_NFS) *fstype = ADIO_NFS;
#endif
	else *fstype = ADIO_UFS;
	*error_code = MPI_SUCCESS;
    }
#elif defined(PARAGON)
    err = statpfs(filename, &ebuf, 0, 0);
    if (err && (errno == ENOENT)) err = statpfs(dir, &ebuf, 0, 0);
    free(dir);

    if (err) *error_code = MPI_ERR_UNKNOWN;
    else {
	if (ebuf.f_type == MOUNT_NFS) *fstype = ADIO_NFS;
	else if (ebuf.f_type == MOUNT_PFS) *fstype = ADIO_PFS;
	else *fstype = ADIO_UFS;
	*error_code = MPI_SUCCESS;
    }
#elif defined(SX4)
     err = stat (filename, &sbuf);
     if (err && (errno == ENOENT)) err = stat (dir, &sbuf);
     free(dir);
 
     if (err) *error_code = MPI_ERR_UNKNOWN;
     else {
         if (!strcmp(sbuf.st_fstype, "nfs")) *fstype = ADIO_NFS;
         else *fstype = ADIO_SFS;
        *error_code = MPI_SUCCESS;
     }
#elif defined(_WIN32)
	*fstype = ADIO_NTFS;
	*error_code = MPI_SUCCESS;
	return;
#else
    /* on other systems, make NFS the default */
	fprintf (stderr, "ad_fstype.c: using NFS as the default file system\n");
	free(dir);
	*fstype = ADIO_NFS;   
	*error_code = MPI_SUCCESS;
#endif

	/* always use ad_svm if available and not specified differently via fs:filename	*/
#if defined(SVM)
	*fstype = ADIO_SVM;
# endif
}
