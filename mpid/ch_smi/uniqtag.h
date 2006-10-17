#ifndef MPID_SMI_SSIDE_GET_UNIQ_TAG_H
#define MPID_SMI_SSIDE_GET_UNIQ_TAG_H




#if 0
#define MPID_SMI_INTERNAL_TAG_LB	(MPID_TAG_UB + 256)

#define MPID_SMI_WIN_CREATE_TAG		(MPID_SMI_INTERNAL_TAG_LB + 1)
#define MPID_SMI_INTERNAL_START_TAG	(MPID_SMI_INTERNAL_TAG_LB + 8)
#else
#define MPID_SMI_INTERNAL_TAG_LB	(MPID_TAG_UB - 256)
#define MPID_SMI_INTERNAL_TAG_UB	(MPID_TAG_UB)

#define MPID_SMI_WIN_CREATE_TAG		(MPID_SMI_INTERNAL_TAG_LB + 1)
#define MPID_SMI_INTERNAL_START_TAG	(MPID_SMI_INTERNAL_TAG_LB + 8)
#endif

int MPID_SMI_Get_uniq_tag (void);
int MPID_SMI_Uniq_tag_init (void);
void MPID_SMI_Uniq_tag_destroy (void);















#endif	/* MPID_SMI_SSIDE_GET_UNIQ_TAG_H */


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
