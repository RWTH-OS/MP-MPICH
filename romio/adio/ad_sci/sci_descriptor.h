//
// $Id$ 
//
// Management of SCI device descriptors:
//  Amount of descriptors is limited, and because each descriptor can be used
//  for one local and one remote segment, we manage them here to make best use
//  of them.
//

#include <list>
#include <sisci_api.h>

#include "sci_priv.h"

class SCI_descriptor {
public:
	SCI_descriptor ();
	~SCI_descriptor ();
    
	// get a descriptor
	sci_desc_t get_local();
	sci_desc_t get_rmt();

	// release a descriptor
	void release (sci_desc_t devdec);
	void release_local (sci_desc_t devdec);
	void release_rmt (sci_desc_t devdec);
  
private:
	// descriptors which are only used for local, remote or both type of resources
	slist<sci_desc_t> only_local_desc;
	slist<sci_desc_t> only_rmt_desc;
	slist<sci_desc_t> complete_desc;
	slist<sci_desc_t> freed_once_desc;

	pthread_mutex_t access_lock;
	bool have_initialized;   // this instance has called SCIInitialize()
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
