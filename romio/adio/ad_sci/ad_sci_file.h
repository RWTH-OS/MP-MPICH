//
// $Id$
//
// AD_SCI_file is the "master class" which contains all methods to access a 
// SCI-distributed file.
//

#include <map>

#include "adio.h"
#include "ad_sci.h"

class AD_SCI_file {
public:
    AD_SCI_file(ADIO_File fd, int *error_code);
    ~AD_SCI_file(int *error_code);
    
    // ADIO functions for this MPI-IO file
    ADIO_Offset SeekIndividual (ADIO_File fd, ADIO_Offset offset, int whence, int *error_code);
    void Fcntl (int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code); 
    void SetInfo (MPI_Info users_info, int *error_code);
    void Flush (int *error_code); 
    void Resize (ADIO_Offset size, int *error_code);

    void ReadContig (void *buf, int count, MPI_Datatype datatype, int file_ptr_type, 
		     ADIO_Offset offset, ADIO_Status *status, int *error_code);
    void WriteContig (void *buf, int count, MPI_Datatype datatype, int file_ptr_type, 
		      ADIO_Offset offset, ADIO_Status *status, int *error_code);  
    void ReadStrided (void *buf, int count, MPI_Datatype datatype, int file_ptr_type,
		      ADIO_Offset offset, ADIO_Status *status, int *error_code);
    void WriteStrided (void *buf, int count, MPI_Datatype datatype, int file_ptr_type,
		       ADIO_Offset offset, ADIO_Status *status, int *error_code);

    void IreadContig (void *buf, int count, MPI_Datatype datatype, int file_ptr_type, 
		      ADIO_Offset offset, ADIO_Request *request, int *error_code);
    void IwriteContig (void *buf, int count, MPI_Datatype datatype, int file_ptr_type, 
		       ADIO_Offset offset, ADIO_Request *request, int *error_code);
    void IreadStrided (void *buf, int count, MPI_Datatype datatype, int file_ptr_type,
		       ADIO_Offset offset, ADIO_Request *request, int *error_code);
    void IwriteStrided (void *buf, int count, MPI_Datatype datatype, int file_ptr_type,
			ADIO_Offset offset, ADIO_Request *request, int *error_code);

    int ReadDone (ADIO_Request *request, ADIO_Status *status, int *error_code); 
    int WriteDone (ADIO_Request *request, ADIO_Status *status, int *error_code);
    void ReadComplete (ADIO_Request *request, ADIO_Status *status, int *error_code); 
    void WriteComplete (ADIO_Request *request, ADIO_Status *status, int *error_code); 

    void ReadStridedColl (void *buf, int count, MPI_Datatype datatype, int file_ptr_type,
			  ADIO_Offset offset, ADIO_Status *status, int *error_code);
    void WriteStridedColl (void *buf, int count, MPI_Datatype datatype, int file_ptr_type,
			   ADIO_Offset offset, ADIO_Status *status, int *error_code);

private:
    ADIO_File fd;

    // member objects 
    SCI_hash     *global_hash;
    SCI_hash     *local_hash;
    SCI_lock     *hash_locks;
    SCI_msg      *msg;
    SCI_transfer *transfer;
    SCI_memaccessor memaccess;
    SCI_memmanager *frgmtmem;
    SCI_memmanager *tagmem;
    SCI_bintree   bintree;

    map< ADSCI_FRGMT_KEY_T, AD_SCI_fragment, less<ADSCI_FRGMT_KEY_T> > loc_frgmts;

    // member functions
}




// 
// Overrides for XEmacs and vim so that we get a uniform tabbing style.
// XEmacs/vim will notice this stuff at the end of the file and automatically
// adjust the settings for this buffer only.  This must remain at the end
// of the file.
// ---------------------------------------------------------------------------
// Local variables:
// c-indent-level: 3
// c-basic-offset: 3
// tab-width: 3
// End:
// vim:tw=0:ts=3:wm=0:
// 
