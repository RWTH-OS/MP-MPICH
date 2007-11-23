//
// $Id: sci_hash.h 889 2001-05-10 18:46:46Z joachim $
//
// Hash-table distributed across multiple SCI memory segments
//

template <class storage> 
class SCI_hash {
public:
  SCI_hash (SCI_memaccessor& sci_memacc, int hash_key);
  ~SCI_hash ();
  
private:

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
