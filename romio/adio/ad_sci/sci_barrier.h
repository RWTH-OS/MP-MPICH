//
// $Id: sci_barrier.h 889 2001-05-10 18:46:46Z joachim $
//
// Inter-process barriers based on SCI fetch-and-add memory
//

class SCI_barrier {
public:
  SCI_barrier (SCI_macc macc);
  ~SCI_barrier ();

  // this is the 'normal' blocking call
  void wait();

  // enter-leave barrier
  void enter();
  void leave();
  
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
