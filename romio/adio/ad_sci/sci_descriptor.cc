//
// $Id: sci_descriptor.cc 889 2001-05-10 18:46:46Z joachim $ 
//
// Management of SCI device descriptors:
//  Amount of descriptors is limited, and because each descriptor can be used
//  for one local and one remote segment, we manage them here to make best use
//  of them. Only one class of type SCI_descriptor should be instantiated by each
//  application for a most effective use of SCI device descriptors.
//
// The current implementation is not yet optimal because it does not know if a 
// descriptor to-be-released releases a local or remote resource. This could be
// implemented in different ways. For now, we just put such a descriptor in the 
// "freed_once" list where he remains until he is freed again, or the user calls
// the specific release_rmt() / release_local() functions.

SCI_descriptor:SCI_descriptor()
{
	sci_error_t sisci_error;
	sci_desc_t sisci_fd;

	// Do we need to initialize? This check is required for the case of 
	// multiple instantiation.
	SCIOpen (&sisci_fd, NO_FLAGS,  &sisci_error);
	if (sisci_error == SCI_ERR_NOT_INITIALIZED) {
      SCIInitialize (0, &sisci_error);      
      CHECK_SISCI_ERROR (sisci_error);
      
      have_initialized = true;
	} else {
      SCIClose (sisci_fd, NO_FLAGS, &sisci_error);
      CHECK_SISCI_ERROR (sisci_error);

      have_initialized = false;
	}
 
	pthread_mutex_init (&access_log, NULL);

	return;
}

SCI_descriptor:~SCI_descriptor()
{
	sci_error_t sisci_error;
	slist<sci_desc_t>::iterator desc;

	// release all remaining descriptors
	for (desc = only_local_desc.begin(); desc != only_local_desc.end(); desc++) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
	}
	for (desc = freed_once_desc.begin(); desc != freed_once_desc.end(); desc++) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
	}
	for (desc = only_rmt_desc.begin(); desc != only_rmt_desc.end(); desc++) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
	}
	for (desc = complete_desc.begin(); desc != complete_desc.end(); desc++) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
	}

	if (have_initialized)
		SCITerminate();

	pthread_mutex_destroy (&access_log);
	return;
}

sci_desc_t SCI_descriptor::get_local()
{
	sci_error_t sisci_error;
	slist<sci_desc_t>::iterator desc = only_rmt_desc.begin();
	sci_desc_t sisci_fd;

	LOCK (&access_lock);

	// check for available remote-only descriptor
	if (desc != only_rmt_desc.end()) {
		only_rmt_desc.pop_front();
		complete_desc.push_front(*desc);

		UNLOCK (&access_lock);
		return *desc;
	}
  
	// none found -> open a new one
	SCIOpen (&sisci_fd, SCI_FLAG_THREAD_SAFE, &sisci_error);
	CHECK_SISCI_ERROR (sisci_error);
	only_local_desc.push_front (sisci_fd);

	UNLOCK (&access_lock);
	return sisci_fd;
}

sci_desc_t SCI_descriptor::get_rmt()
{
	sci_error_t sisci_error;
	slist<sci_desc_t>::iterator desc = only_local_desc.begin();
	sci_desc_t sisci_fd;

	LOCK (&access_lock);

	// check for available local-only descriptor
	if (desc != only_local_desc.end()) {
		only_local_desc.pop_front();
		complete_desc.push_front(*desc);
		UNLOCK (&access_lock);
		return *desc;
	}
  
	// none found -> open a new one
	SCIOpen (&sisci_fd, SCI_FLAG_THREAD_SAFE, &sisci_error);
	CHECK_SISCI_ERROR (sisci_error);
	only_rmt_desc.push_front (sisci_fd);

	UNLOCK (&access_lock);
	return sisci_fd;
}

void SCI_descriptor::release (sci_desc_t devdec)
{
	slist<sci_desc_t>::iterator desc;
	sci_error_t sisci_error;
	sci_desc_t sisci_fd;

	LOCK (&access_lock);

	// find the list in which the descriptor is stored
	desc = find (complete_desc.begin(), complete_desc.end(), devdec);
	if (desc != complete_desc.end()) {
		freed_once_desc.push_front (*desc);
		complete_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	desc = find (only_rmt_desc.begin(), only_rmt_desc.end(), devdec);
	if (desc != only_rmt_desc.end()) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
		only_rmt_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	desc = find (only_local_desc.begin(), only_local_desc.end(), devdec);
	if (desc != only_local_desc.end()) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
		only_local_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	desc = find (freed_once_desc.begin(), freed_once_desc.end(), devdec);
	if (desc != freed_once_desc.end()) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
		freed_once_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	// invalid device descriptor - exception? Anyway, we just quit.
	UNLOCK (&access_lock);
	return;
}

void SCI_descriptor::release_rmt (sci_desc_t devdec)
{
	slist<sci_desc_t>::iterator desc;
	sci_error_t sisci_error;
	sci_desc_t sisci_fd;

	LOCK (&access_lock);

	// find the list in which the descriptor is stored
	desc = find (complete_desc.begin(), complete_desc.end(), devdec);
	if (desc != complete_desc.end()) {
		only_local_desc.push_front (*desc);
		complete_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	desc = find (only_rmt_desc.begin(), only_rmt_desc.end(), devdec);
	if (desc != only_rmt_desc.end()) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
		only_rmt_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	// invalid device descriptor - exception? Anyway, we just quit.
	UNLOCK (&access_lock);
	return;
}

void SCI_descriptor::release_local (sci_desc_t devdec)
{
	slist<sci_desc_t>::iterator desc;
	sci_error_t sisci_error;
	sci_desc_t sisci_fd;

	LOCK (&access_lock);

	// find the list in which the descriptor is stored
	desc = find (complete_desc.begin(), complete_desc.end(), devdec);
	if (desc != complete_desc.end()) {
		only_rmt_desc.push_front (*desc);
		complete_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	desc = find (only_local_desc.begin(), only_local_desc.end(), devdec);
	if (desc != only_local_desc.end()) {
		SCIClose (*desc, NO_FLAGS, &sisci_error);
		CHECK_SISCI_ERROR (sisci_error);
		only_local_desc.erase (*desc);

		UNLOCK (&access_lock);
		return;
	}

	// invalid device descriptor - exception? Anyway, we just quit.
	UNLOCK (&access_lock);
	return;
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
