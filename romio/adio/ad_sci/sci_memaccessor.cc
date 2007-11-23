//
// $Id: sci_memaccessor.cc 891 2001-05-14 17:54:22Z joachim $
//
// Access memory from local or remote SCI segments
//

SCI_memaccessor::SCI_memaccessor (int adapter)
{
    sci_desc_t sisci_fd;
    sci_error_t sisci_error;
    sci_query_adapter_t sisci_adpt_q;
    int q_data;

    // Determine if specified SCI adapter can be used
    sisci_adpt_q.localAdapterNo = (adapter >= 0 ? adapter : 0);
    sisci_adpt_q.portNo = 0;
    sisci_adpt_q.subcommand = SCI_Q_ADAPTER_CONFIGURED;
    sisci_adpt_q.data = (void *)&q_data;
    SCIQuery (SCI_Q_ADAPTER, (void *)&sisci_adpt_q, NO_FLAGS, &sisci_error);
    CHECK_SISCI_ERROR (sisci_error);    
    if (q_data == 0) {
		 fprintf (stderr, "SCI_descriptor: specified SCI adapter %d is not available\n",  
					 sisci_adpt_q.localAdapterNo);
		 // XXX exception
    }
	 adapter_nbr = adapter;
	 
	 // Determine the node id of this adapter
	 sisci_adpt_q.localAdapterNo = adapter_nbr;
    sisci_adpt_q.portNo = 0;
    sisci_adpt_q.subcommand = SCI_Q_ADAPTER_NODEID;
    sisci_adpt_q.data = (void *)&local_node_id;
    SCIQuery (SCI_Q_ADAPTER, (void *)&sisci_adpt_q, NO_FLAGS, &sisci_error);
    CHECK_SISCI_ERROR (sisci_error);    

	 // we need to know the local page size for alignment of size & offset for mappings
	 pagesize = (int)sysconf(_SC_PA2GESIZE);

	 pthread_mutex_init (&access_lock, NULL);
    
    return;
}

SCI_memaccessor::~SCI_memaccessor ()
{
	// disconnect all remote segments
	hash_map<ulong, SCI_sgmt_info, hash<ulong>, equal_to<ulong> >::iterator sgmt_it;
	for (sgmt_it = sci_memory_sgmts.begin(); sgmt_it != sci_memory_sgmts.end(); sgmt_it++)
		disconnect_sgmt (*sgmt_it);
	
	// remove all local segments
	SCI_connector cnct;
	for (sgmt_it = sci_memory_sgmts.begin(); sgmt_it != sci_memory_sgmts.end(); sgmt_it++) {
		cnct.node_id = (*sgmt_it).node_id;
		cnct.sgmt_id = (*sgmt_it).sgmt_id;
		cnct.adapter = adapter_nbr;
		remove_shmem (cnct&);
	}

	pthread_mutex_destroy (&access_lock);
	
	return;
}

bool SCI_memaccessor::put (SCI_macc macc, void *buf, size_t len, uint flags)
{
	if (len == 0)
		return true;

	LOCK (&access_lock);
	GET_SGMT_INFO (macc, len, flags);
	
	if (sgmt->disconnect_requested) {
		UNLOCK (&access_lock);
		return false;
	}

	// block until a callback makes the segment accessible again.
	while (!sgmt_info.accessable) {
		pthread_cond_wait (&sgmt_info.access_cv, &access_lock);
	}
	
	sci_error_t sisci_err;
	SCIMemCopy (buf, sgmt_info.map, macc.l.offset*macc.stride - sgmt_info.offset, len, 
					SCI_FLAG_ERROR_CHECK, &sisci_err);
	if (sisci_err != SCI_ERR_OK)
		return false;

	UNLOCK (&access_lock);
	return;
}

bool SCI_memaccessor::get (SCI_macc macc, void *buf, size_t len, uint flags)
{
	if (len == 0)
		return true;

	LOCK (&access_lock);
	GET_SGMT_INFO (macc, len, flags);

	if (sgmt->disconnect_requested) {
		UNLOCK (&access_lock);
		return false;
	}

	// Check the state of the segment, and start sequence. We could speed up things 
	// here if we positively trust the accessable flag - but this is prone to
	// race conditions. Should we risk it?

	// block until a callback makes the segment accessible again.
	while (!sgmt_info.accessable) {
		pthread_cond_wait (&sgmt_info.access_cv, &access_lock);
	}
	
	sci_error_t sisci_err;
	SCIMemCopy (buf, sgmt_info.map, macc.l.offset*macc.stride - sgmt_info.offset, len, 
					SCI_FLAG_ERROR_CHECK|SCI_FLAG_BLOCK_READ, &sisci_err);
	if (sisci_err != SCI_ERR_OK)
		return false;

#if 0
	// get the address and copy data as indicated by flags  
	void *src = range (macc, len);
	if (src != NULL) {
		do {
			SCI_MEMCPY_R (buf, src, len);
		} while (!sgmt_transfer_ok (sgmt_info));
	} else {
		// XXX exception: can not copy data 
		return false;
	}
#endif

	UNLOCK (&access_lock);
	return true;
}

void *SCI_memaccessor::range (SCI_macc macc, size_t len)
{
	void *addr;

	LOCK (&access_lock);

	// check if already connected
	GET_SGMT_INFO (macc, len, NO_FLAGS);

	// verify that desired range is contained in the segment
	if ((sgmt_info.offset > macc.stride * macc.l.offset)
			 || (sgmt_info.offset + sgmt_info.size < macc.stride * macc.l.offset + len)) {
		// reconnect with sufficient mapping
		sgmt_info = reconnect_sgmt (macc, len);
	} 
	
	addr = (sgmt_info.size > 0) ? (void *)(sgmt_info.addr + macc.l.offset * macc.stride - sgmt_info.offset) : NULL;

	UNLOCK (&access_lock);
	return addr;
}

void *SCI_memaccessor::range_lock (SCI_macc macc, size_t len)
{
	// get range
	void *addr = range (macc, len);
	// XXX this 'split locking' could result in race conditions
	LOCK (&access_lock);

	if (addr != NULL) {
		GET_SGMT_INFO (macc, len, NO_FLAGS);
		
		sgmt_info.lock_cntr++;
	}

	UNLOCK (&access_lock);
	return addr;
}

SCI_memaccessor::lock_release (SCI_macc macc)
{
	LOCK (&access_lock);

	// find segment and decrement lock counter
	GET_SGMT_INFO (macc, len, NO_FLAGS);
	
	if (sgmt_info.lock_cntr > 0) {
		sgmt_info.lock_cntr--;
	} else {
		// error in lock nesting!
		// XXX exception
	}
	
	if ((sgmt_info.lock_cntr == 0) && (*sgmt_it).disconnect_requested) {
		sgmt_info.accessable = false;
		if (!disconnect_sgmt (sgmt_info, false)) {
			// XXX this must not fail -> exception
		}
	}

	UNLOCK (&access_lock);
	return;
}


//
// managing local segments
//
void *SCI_memaccessor::create_shmem (SCI_connector& cnct, int size, uint flags, int adapter)
{
	sci_error_t sisci_err;

	SCI_sgmt_info *sgmt = new SCI_sgmt_info;

	LOCK (&access_lock);

	sgmt->is_local = true;
	sgmt->node_id = local_node_id;
	sgmt->desc = sci_desc.get_local();
	sgmt->cnct_cntr = 0;
	sgmt->lock_cntr = 0;
	sgmt->size = (size % pagesize) ? (size/pagesize + 1)*pagesize : size;
	
	do {
		SCICreateSegment (sgmt->desc, &(sgmt->segment).local, ++last_sgmt_id, sgmt_size,
								this->cb_loc, (void *)sgmt, SCI_FLAG_USE_CALLBACK, &sisci_err);
	} while (sisci_err == SCI_ERR_SEGMENTID_USED);
	CHECK_SISCI_ERROR (sisci_error);
	if (last_sgmt_id >= (1 << MACC_SGMT_WIDTH)) {
		// too many segments for the accessor sgmt width!
		// XXX raise exception
		UNLOCK (&access_lock);
		return NULL;
	}
	sgmt->sgmt_id = last_sgmt_id;

	SCIPrepareSegment ((sgmt->segment).local, adapter_nbr, NO_FLAGS, &sisci_err);
	CHECK_SISCI_ERROR (sisci_error);

	sgmt->addr = (char *)SCIMapLocalSegment ((sgmt->segment).local, &sgmt->map, 0, sgmt->size,
														  NULL,  &sisci_err);
	CHECK_SISCI_ERROR (sisci_error);
	sgmt->atomic_map = NULL;
	memset ((void *)sgmt->addr, 0, sgmt->size);
	
	SCISetSegmentAvailable ((sgmt->segment).local, adapter_nbr, NO_FLAGS, &sisci_err);
	CHECK_SISCI_ERROR (sisci_error);

	sci_memory_sgmts[(ulong)(local_node_id << MACC_PRANK_WIDTH + sgmt->sgmt_id)] = *sgmt_info;
	
	UNLOCK (&access_lock);
	return (void *)sgmt->addr;
}

bool SCI_memaccessor::remove_shmem (SCI_connector& cnct)
{
	sci_error_t sisci_err;
	ulong sgmt_key = (ulong)(cnct.node_id << MACC_PRANK_WIDTH + cnct.sgmt_id);

	LOCK (&access_lock);
	SCI_sgmt_info& sgmt = sci_memory_sgmts[sgmt_key];
	
	if (!sgmt.is_local) {
		UNLOCK (&access_lock);
		return false;
	}

	SCIUnmapSegment (sgmt.map, NO_FLAGS, &sisci_err);
	CHECK_SISCI_ERROR (sisci_err);

	SCISetSegmentUnavailable (sgmt.local.segment, adapter_nbr, SCI_FLAG_NOTIFY, &sisci_err);
	CHECK_SISCI_ERROR (sisci_err);

	do {
		SCIRemoveSegment (sgmt.local.segment, NO_FLAGS, &sisci_err);
	} while (sisci_err == SCI_ERR_BUSY);
	CHECK_SISCI_ERROR (sisci_err);

	sci_desc.release_local (sgmt.desc);
	sci_memory_sgmts.erase[sgmt_key];
	
	UNLOCK (&access_lock);
	return true;
}


//
// managing remote segments
//
SCI_sgmt_info& SCI_memaccessor::connect_sgmt (SCI_macc macc, size_t len, uint flags)
{
	uint map_flags = NO_FLAGS;
	sci_error_t sisci_err;
	sci_map_t *map;
	char **addr;
	SCI_sgmt_info *sgmt;

	LOCK (&access_lock);
	
	// check if we only need to re-map this segment differenty, or if we have to 
	// initially connect & map
	ulong sgmt_key = macc.l.node_id << MACC_PRANK_WIDTH + macc.l.sgmt_id; 
	hash_map<ulong, SCI_sgmt_info, hash<ulong>, equal_to<ulong> >::iterator sgmt_it = 
		find (sci_memory_sgmts.begin(), sci_memory_sgmts.end(), sgmt_key); 
	if (sgmt_it != sci_memory_sgmts.end()) {
		return remap_sgmt (*sgmt_it, len, flags);
	}

	// segment does not yet exist, we need to connect
	sgmt = new SCI_sgmt_info;
	sgmt->is_local = false;
	sgmt->lock_cntr = 0;
	sgmt->cnct_cntr = 0;
	sgmt->disconnect_requested = false;
	sgmt->desc = sci_desc.get_rmt();

	SCIConnectSegment (sgmt->desc, &(sgmt->segment).remote, (uint)macc.l.node_id, 
							 (uint)macc.l.sgmt_id, adapter_nbr, this->cb_rmt, (void *)sgmt,
							 0, SCI_FLAG_USE_CALLBACK, &sisci_err);
	CHECK_SISCI_ERROR (sisci_err);

	sgmt->size = SCIGetRemoteSegmentSize ((sgmt->segment).remote);
	if (sgmt->size < (macc.l.offset * macc.stride) + len) {		
		// segment too small or illegale accessor
		SCIDisconnectSegment ((sgmt->segment).remote, NULL,  &sisci_err);
		sci_desc.release_rmt (sgmt->desc);
		delete SCI_sgmt;

		// XXX raise exception
		return NULL;
	}

	// select which kind of mapping is desired: default, atomic or readonly
	sgmt->is_atomic = false;
	sgmt->is_readonly = false;
	map = &sgmt->map;
	addr = &sgmt->addr;
	if (flags & SCI_ACC_ATOMIC) {
		map_flags |= SCI_FLAG_LOCK_OPERATION;
		map = &sgmt->atomic_map;
		addr = &sgmt->atomic_addr;
		sgmt->is_atomic = true;
	} else {
		if (flags & SCI_ACC_READONLY) {
			map_flags |= SCI_FLAG_READONLY_MAP;
			map = &sgmt->readonly_map;
			addr = &sgmt->readonly_addr;
			sgmt->is_readonly = true;
		}
	}
	// first try to map the whole segment
	*addr = (char *)SCIMapRemoteSegment ((sgmt->segment).remote, map, 
													 0, sgmt->size, NULL, map_flags, &sisci_err);
	if (sisci_err != SCI_ERR_OK) {
		// Probably we are running out of resources - try to map only the part of 
		// the segment which is required right now.
		sgmt->size   = ((len/pagesize) + 1) * pagesize;
		sgmt->offset = (((macc.l.offset * macc.stride)/pagesize) + 1) * pagesize;
		*addr = (char *)SCIMapRemoteSegment ((sgmt->segment).remote, map, sgmt->offset, sgmt->size, 
														 NULL, map_flags, &sisci_err);
		CHECK_SISCI_ERROR (sisci_err);
	}
	
	// create & init sequence
	SCICreateMapSequence (sgmt->map, &sgmt->sequence, NO_FLAGS, &sisci_err);
	CHECK_SISCI_ERROR (sisci_err);
	do {
		// XXX avoid potential endless loop via timeout / max_try counter ?
		sgmt->seq_status = SCIStartSequence (sgmt->sequence, NO_FLAGS, &sisci_err);
	} while (sgmt->seq_status != SCI_SEQ_OK);
		
	// init status control
	pthread_cond_init (&sgmt->access_cv, NULL);
	sgmt->is_accessable = true;
	sci_memory_sgmts[(ulong)(macc.l.prank << MACC_PRANK_WIDTH + macc.l.sgmt_id)] = *sgmt;

	UNLOCK (&access_lock);
	return (*sgmt)&;
}

bool SCI_memaccessor::reconnect_sgmt (SCI_sgmt_info& sgmt, SCI_macc macc, size_t len, uint flags)
{
	if (disconnect_sgmt (sgmt)) {
		connect_sgmt (macc, len, flags);
		return true;
	}
	
	return false;
}

bool SCI_memaccessor::disconnect_sgmt (SCI_sgmt_info& sgmt, bool do_lock)
{
	sci_error_t sisci_err;

	if (do_lock) {
		LOCK (&access_lock);
	}
		
	// only remote segments can be disconnected
	if (sgmt.is_local) {
		if (do_lock) {
			UNLOCK (&access_lock);
		}
		return false;
	}
	// check lock counter
	if (sgmt.lock_cntr > 0) {
		if (do_lock) {
			UNLOCK (&access_lock);
		}
		return false;
	}

	SCIRemoveSequence (sgmt.sequence, NO_FLAGS, &sisci_err);
	CHECK_SISCI_ERROR (sisci_err);
	
	SCIUnmapSegment (sgmt.map, NO_FLAGS, &sisci_err);
	CHECK_SISCI_ERROR (sisci_err);

	SCIDisconnectSegment (sgmt.segment.remote, NO_FLAGS,  &sisci_err);
	// XXX could this return SCI_ERR_BUSY?
	CHECK_SISCI_ERROR (sisci_err);

	pthread_mutex_destroy (&sgmt->access_lock);
	pthread_cond_destroy (&sgmt->access_cv);
	sci_memory_sgmts.erase (sgmt.node_id << MACC_PRANK_WIDTH + sgmt.sgmt_id);
	delete sgmt;  // is this required?

	if (do_lock) {
		UNLOCK (&access_lock);
	}
	return true;
}


//
// segment callback functions
//
sci_callback_action_t SCI_memaccessor::cb_rmt (void *arg, sci_remote_segment_t segment, 
															  sci_segment_cb_reason_t reason, sci_error_t status)
{
	// If the remote process withdraws his segment, we need to disconnect from it. If this
	// segment is locked, we have a problem - if we just disconnect it, we risk a SEGV. If 
	// we don't, the remote process is blocked while trying to remove his segment. But this
	// is better than a SEGV - therefore, we mark the segment to be disconnected so that it
	// will be disconnect immeadeletly if its lock_cntr is 0.
	SCI_sgmt_info *sgmt = (SCI_sgmt_info *)arg;

	LOCK (&access_lock);

	switch (reason) {
	case SCI_CB_DISCONNECT: 		
		if (sgmt->lock_cntr > 0) {
			sgmt->disconnect_requested = true;
			UNLOCK (&access_lock);			
			return SCI_CALLBACK_CONTINUE;
		}
		sgmt->accessable = false;
		if (!disconnect_sgmt (*sgmt, false)) {
			// XXX this must not fail! -> exception
		}
		UNLOCK (&access_lock);			
		return SCI_CALLBACK_CANCEL;
		
	case SCI_CB_NOT_OPERATIONAL:
		sgmt->accessable = false;
		UNLOCK (&access_lock);	
		break;

	case SCI_CB_OPERATIONAL:
		sgmt->accessable = true;
		UNLOCK (&access_lock);
		// inform potentially waiting threads on the event
		pthread_cond_broadcast (&sgmt->access_cv);
		break;

	case SCI_CB_LOST:		
		sgmt->accessable = false;
		if (sgmt->lock_cntr > 0) {
			sgmt->disconnect_requested = true;
			UNLOCK (&access_lock);			
			return SCI_CALLBACK_CANCEL;
		}
		if (!disconnect_sgmt (*sgmt, false)) {
			// XXX this must not fail! -> exception
		}
		UNLOCK (&access_lock);			
		return SCI_CALLBACK_CANCEL;

	default:
		break;
	}
	return SCI_CALLBACK_CONTINUE;
}

sci_callback_action_t SCI_memaccessor::cb_loc (void *arg, sci_local_segment_t segment,
															  sci_segment_cb_reason_t reason, unsigned int nodeId, 
															  unsigned int localAdapterNo, sci_error_t error)
{
	SCI_sgmt_info *sgmt = (SCI_sgmt_info *)arg;

	LOCK (&access_lock);

	switch (reason) {
	case SCI_CB_CONNECT:
		sgmt->cnct_cntr++;
		break;

	case SCI_CB_DISCONNECT:
		sgmt->cnct_cntr--;
		break;

	case SCI_CB_OPERATIONAL:
		sgmt->is_accessable = true;
		break;
	
	case SCI_CB_NOT_OPERATIONAL:
		sgmt->is_accessable = false;
		break;
		
	case SCI_CB_LOST:
		// The remote node will not be able to disconnect - 
		// but the connection is lost. 
		sgmt->cnct_cntr--;
		break;
	}
	 
	UNLOCK (&access_lock);
	return SCI_CALLBACK_CONTINUE;
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
