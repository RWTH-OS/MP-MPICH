//
// $Id$
//
// A fragment is a contignous piece of the file located on one node
//

#include <unistd.h>

#include <list>

#include "adio.h"
#include "ad_sci.h"

// this is the local representation of a file fragment 
class AD_SCI_fragment {
public:
  // constructors / destructors
  AD_SCI_fragment (SCI_memlocator &ml, bool all_readers = false);
  AD_SCI_fragment (int file_des);

  ~AD_SCI_fragment(int file_des);  
  ~AD_SCI_fragment();  
  
  ADSCI_HASH_KEY_T getkey();

private:
  SCI_fragment frgmt;
  list<int> *readers;
  
  int   loc_filedes;
  off_t loc_offset;  
}

// This is how file fragments are actually stored in SCI shared memory
// (the marshalled version of AD_SCI_fragment). The size of this struct
// is very critical, it must be an "aligned" SCI size like 64 or 128 byte.
struct sci_frgmt {
  SCI_mloc tree_left, tree_right;
  SCI_mloc file_next, file_prev;

  short owner;  
  ADIO_Offset offset, len;
  ADIO_Offset lock_offset, lock_len;
    
  SCI_mloc more_readers;
};

#define ADSCI_FRGMT_NBRREADERS ((ADSCI_FRGMT_SIZE - sizeof(struct sci_frgmt))/sizeof(short))

typedef struct _SCI_fragment {
  struct sci_frgmt data;
  short readers[ADSCI_FRGMT_NBRREADERS];
} SCI_fragment;





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
