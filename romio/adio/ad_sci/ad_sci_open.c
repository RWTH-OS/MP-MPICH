/* 
 *   $Id: ad_sci_open.c 869 2001-04-25 15:23:43Z joachim $    
 *
 *   
 *   
 */

#include "ad_sci.h"

void ADIOI_SCI_Open(ADIO_File fd, int *error_code)
{
    int perm, old_mask, amode;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_SCI_OPEN";
#endif

    MPI_Comm_size(fd->comm, &_adsci_nprocs);
    MPI_Comm_rank(fd->comm, &_adsci_myrank);

    if (fd->perm == ADIO_PERM_NULL) {
	old_mask = umask(022);
	umask(old_mask);
	perm = old_mask ^ 0666;
    }
    else perm = fd->perm;

    amode = 0;
    if (fd->access_mode & ADIO_CREATE)
	amode = amode | O_CREAT;
    if (fd->access_mode & ADIO_RDONLY)
	amode = amode | O_RDONLY;
    if (fd->access_mode & ADIO_WRONLY)
	amode = amode | O_WRONLY;
    if (fd->access_mode & ADIO_RDWR)
	amode = amode | O_RDWR;
    if (fd->access_mode & ADIO_EXCL)
	amode = amode | O_EXCL;

    // set up SCI resources (memory, locks, interrupts, callbacks, ...) 
    _adsci_SCI_init();

    /* Is the file already open? If yes, copy the relevant attributes and return */
    
#if 0
     /* where is the file? ask the server and let him copy the relevant parts to my disk */

#else
    /* for now: only create, read & write files locally */
    
#endif

    /* APPEND mode? -> set file ptr to end of file */

#ifdef PRINT_ERR_MSG
    *error_code = (fd->fd_sys == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
#else
    if (fd->fd_sys == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(ADIO_FILE_NULL, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
}
