#ifndef MPID_SMI_SSIDE_MEMCPY_H
#define MPID_SMI_SSIDE_MEMCPY_H



#define MPID_SMI_MEMCPY_DEST_LOCAL      1
#define MPID_SMI_MEMCPY_SRC_LOCAL       2
#define MPID_SMI_MEMCPY_LOCAL_SHARED    4


int MPID_SMI_Sside_memcpy (void *, void *, size_t, int, int, int*);















#endif	/* MPID_SMI_SSIDE_MEMCPY_H */


/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
