/* $Id$
   use the SISCI builtin function for memory transfers */

#include <string.h>

#include "env/general_definitions.h"
#include "regions/address_to_region.h"
#include "env/smidebug.h"
#include "memcpy_base.h"

#ifndef NO_SISCI
void *_smi_sisci_memcpy(void *dest, const void *src, size_t len)
{
    shseg_t *rmtseg;
    sci_map_t rmtmap = NULL;
    sci_error_t sisci_err;
    int cpy_len=len;
    
    rmtseg = _smi_addr_to_shseg(dest);
    if ((rmtseg != NULL) && (rmtseg->segment != NULL)) {
	rmtmap = rmtseg->map;
    } 
		
    if (rmtmap == NULL) {
	memcpy (dest, src, len);
    } else {
#if HAVE_SCIMEMCOPY
	/* SCIMemCopy() needs 4-byte size alignment */
	cpy_len = len - (len%4);
	do {
	    SCIMemCopy((void *)src, rmtmap, ((unsigned long)dest) - ((unsigned long)rmtseg->address),
		       cpy_len, 0 /* SCI_FLAG_ERROR_CHECK */, &sisci_err);
	} while (sisci_err == SCI_ERR_TRANSFER_FAILED);
#else
	sisci_err = SCI_ERR_FLAG_NOT_IMPLEMENTED;
#endif
	if (sisci_err != SCI_ERR_OK) {
	    DPROBLEMP ("SCIMemCopy() failed, SISCI error", sisci_err);
	    memcpy (dest, src, len);
	} else {
	    /* copy the potential rest */
	    if (len%4) {
		memcpy (((char *)dest)+cpy_len, ((char *)src)+cpy_len, len%4);
	    }
	}
    }
    return dest;
}

#else
void *_smi_sisci_memcpy(void *dest, const void *src, size_t len)
{
    return memcpy(dest, src, len);
}
#endif
