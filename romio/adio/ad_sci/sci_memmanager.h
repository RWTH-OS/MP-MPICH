//
// $Id: sci_memmanager.h,v 1.2 2001/05/10 18:46:46 joachim Exp $
//
// Simple memory manager for local SCI segments:
//  - manages only blocks of fixed size
//  - can only be called by a single, local process
//

#include <map>

#include "sisci_api.h"

#include "scipriv.h"

//
// abstract base class
//
class SCI_memmanager {
public:
  virtual SCI_memmanager (size_t stride = 64, int sgmt_size = 1024*1024, int adapter = 0);
  virtual ~SCI_memmanager ();

  virtual SCI_mloc alloc ();
  virtual void free (SCI_mloc mem);
}

//
// allocator for single blocks only
//
class SCI_memmanager_single::SCI_memmanager {
public:
  SCI_memmanager (size_t stride = 64, int sgmt_size = 1024*1024, int adapter = 0);
  ~SCI_memmanager ();

  SCI_mloc alloc ();
  void free (SCI_mloc mem);

private:
  map< int, void *, less<int> > sci_sgmt_addrs;  // addresses of local SCI segments, referenced by their ID
  vector< int > sci_sgmt_ids;                    // IDs of all local SCI segments

  stack< SCI_mloc > freelist;
  hash_set< SCI_mloc > allocated;
}

//
// allocator for multiple contignous blocks, too
//
class SCI_memmanager_multi::SCI_memmanager {
public:
  virtual SCI_memmanager (size_t stride = 64, int sgmt_size = 1024*1024, int adapter = 0);
  ~SCI_memmanager ();

  SCI_mloc alloc (int count = 1);
  void free (SCI_mloc mem);

private:
  map< int, void *, less<int> > sci_sgmt_addrs;  // addresses of local SCI segments, referenced by their ID
  vector< int > sci_sgmt_ids;                    // IDs of all local SCI segments

  set< SCI_mloc > freelist;
  hash_set< SCI_mloc > allocated;
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
