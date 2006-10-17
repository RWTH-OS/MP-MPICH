#include <stdlib.h>
#include <mpi.h>
#include <smi.h>

#include "mpiimpl.h"
#include "mpid.h"
#include "adi3types.h"
#include "smidev.h"
#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"
#include "smimem.h"
#include "sside_memcpy.h"


#define MPID_SMI_SSIDE_S_SIZE	64		/* 64 B */


int MPID_SMI_Sside_memcpy (dest, src, n, drank, cpflag, verify)
	void	* dest;
	void	* src;
	size_t	n;
	int		drank;	/* remote device rank */
	int		cpflag;
	int		* verify;
{
	int					region_id, sgmt_id;
	smi_region_info_t	region_info;
	smi_memcpy_handle	handle;
	void				* remote_addr;
	int					scpflag;	
	int					write;
	int					local_isshared;
	int					local2local, rem2rem;


	write = !(cpflag & MPID_SMI_MEMCPY_DEST_LOCAL);
	local2local = (cpflag & MPID_SMI_MEMCPY_DEST_LOCAL) && 
					(cpflag & MPID_SMI_MEMCPY_SRC_LOCAL);
	local_isshared = (cpflag & MPID_SMI_MEMCPY_LOCAL_SHARED);
	rem2rem = !(cpflag & MPID_SMI_MEMCPY_DEST_LOCAL) &&
				!(cpflag & MPID_SMI_MEMCPY_SRC_LOCAL);
	
	if (local2local) {
		memcpy (dest, src, n);
		return 1;
	}

	if (rem2rem) return 0;

#if 0
	/* XXX: DMA transfer - not yet tested ! */
	if (MPID_SMI_cfg.USE_DMA && n > MPID_SMI_cfg.DMA_MINSIZE && MPID_SMI_cfg.REGISTER) {
		if (local_isshared) {
			if (write) {
				scpflag = SMI_MEMCPY_LS_RS;
			} else {
				scpflag = SMI_MEMCPY_RS_LS;
			}
		} else {
			if (write) {
				remote_addr = dest;
				scpflag = SMI_MEMCPY_LP_RS;
			} else {
				remote_addr = src;
				scpflag = SMI_MEMCPY_RS_LP;
			}
			/* we need to register our memory */
			err = MPID_SMI_Local_mem_register(remote_addr, n, &region_id, &sgmt_id);
			MPID_ASSERT (err == MPI_SUCCESS, "Could not register memory for one-sided DMA operation");			
		}
		handle = NULL;
		SMIcall (SMI_Imemcpy (dest, src, (size_t) n, scpflag, &handle));
		/* wait for finish */
		SMIcall (SMI_Memwait (handle));
		/* unregister the memory again */
		MPID_SMI_Local_mem_release (NULL, region_id, MPID_SMI_RSRC_DESTROY);
	} else if (n > MPID_SMI_SSIDE_S_SIZE) {
#else
	if (n > MPID_SMI_SSIDE_S_SIZE) {
#endif
		if (write) {
#if 0
			MEMCPY_W (dest, src, n, drank);
			*verify = 0;
#else
			MPID_STAT_CALL(sside_memcpy);
			REMOTE_COPY (dest, src, n);
			MPID_STAT_RETURN(sside_memcpy);
			*verify = 1;
#endif
		} else {
			MPID_STAT_CALL(sside_memcpy);
			MEMCPY_R (dest, src, n);
			MPID_STAT_RETURN(sside_memcpy);
			*verify = 1;
		}
	} else {
		MPID_STAT_CALL(sside_memcpy);
		MEMCPY_S (dest, src, n);
		MPID_STAT_RETURN(sside_memcpy);
		*verify = 1;
	}
	return 1;
}























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
