//
// $Id$
//

//
// public functions
//

SCI_msg::SCI_msg (vector<SCI_macc>& info_macc,  int rank, SCI_memaccessor& sci_memacc, 
						SCI_descriptor& sci_desc, int pkt_size, int nbrpkts)
{
	SCI_connector cnct;
	SCI_macc msg_macc;
	SCI_msgqueue msg_queue;
	char *local_packets;
	sci_error_t sisci_err;
	sci_desc_t intrpt_desc;
	sci_remote_interrupt_t rmt_intrpt;

	desc = sci_desc;
	memacc = sci_memacc;

	msg_out.reserve (sci_cnct.size());
	msg_in.reserve (sci_cnct.size());

	pkt_size  = (pktsize > 0) ? pktsize : SCI_MSG_DEFAULT_PKTSIZE;
	nbr_pkts  = (nbrpkts > 0) ? nbrpkts : SCI_MSG_DEFAULT_NBRPKTS;
	nbrprocs = queue_macc.size();
	myrank   = rank;

	// create local resources (interrupt & shared memory for incoming msgs)
	local_intrpt_desc = desc.get_local();
	SCICreateInterrupt (local_intrpt_desc, &local_intrpt, memacc.local_adapter(), &local_intrpt_nbr, 
							  this->msg_arrived, NULL, SCI_FLAG_USE_CALLBACK, &sisci_err);
	CHECK_SISCI_ERROR (sisci_err);
	cnct.intrpt_id = local_intrpt_nbr;
	
	uint mem_per_proc = slotsize*(nbr_slots + 1);  // one slot for read-msg counter
	cnct = memacc.create_shmem (nbrprocs*mem_per_proc, NO_FLAGS);
	MACC_INIT (msg_macc, cnct.node_id, cnct.sgmt_id, 0, 1);
	local_packets = (char *)memacc.range_lock (msg_macc, nbrprocs*mem_per_proc);
	payload = pkt_size - (sizeof(SCI_packet) - sizeof(char));


	// init queues for incoming messages
	for (int p = 0; p < nbrprocs; p++) {
		msg_queue.base = static_cast<SCI_packet *>(local_packets + p*mem_per_proc);
		msg_queue.next = msg_queue.base;
		msg_queue.last = static_cast<SCI_packet *>(local_packets + p*mem_per_proc + (nbr_slots - 1)*slotsize);
		msg_queue.intrpt.local = local_intrpt;
		msg_queue.id = 1;
		msg_queue.nbr_msgs = 0;
		msg_queue.msg_read_offset = p*mem_per_proc + slotsize*nbr_slots;
		msg_queue.nbr_slots = nbr_slots;
		
		msg_in.push_back (msg_queue);
	}

	// let the remote processes connect to our queues
	for (vector<SCI_macc>::iterator macc_it = info_macc.begin();
		  macc_it != info_macc.end(); macc_it++) {
		memacc.put (*macc_it, sizeof(SCI_connector), &cnct, NO_FLAGS);
	}

	// poll the local memory for connectors to remote memory (poll for node_id != 0)
	SCI_connector *rmt_cnct = 
		(SCI_connector *)memacc.range_lock (info_macc[myrank], nbrprocs*sizeof(SCI_connector));
	for (int p = 0; p < nbrprocs; p++) {
		if (p == myrank) {
			queue_mem_cnct.push_back(cnct);
			continue;
		}
		while (rmt_cnct[p].node_id == 0)
			usleep(100);
		
		queue_mem_cnct.push_back(rmt_cnct[p]);
	}

	// lock the addresses of the queues in remote memory and set up the local ptrs
	for (int p = 0; p < nbrprocs; p++) {
		MACC_INIT (msg_macc, queue_mem_cnct[p].node_id,  queue_mem_cnct[p].sgmt_id, 
					  myrank*mem_per_proc, 1);

		char *queue_addr = static_cast<char *>(memacc.range_lock (msg_macc, mem_per_proc));
		msg_queue.base = static_cast<SCI_packet *>(queue_addr);
		msg_queue.next = msg_queue.base;
		msg_queue.last = static_cast<SCI_packet *>(queue_addr + (nbr_slots - 1)*slotsize);

		intrpt_desc = desc.get_remote();
		SCIConnectInterrupt(intrpt_desc, &rmt_intrpt, queue_mem_cnct[p].node_id, memacc.local_adapter(),
								  queue_mem_cnct[p].intrpt_id, 0, NO_FLAGS, &sisci_err);
		CHECK_SISCI_ERROR (sisci_err);
		
		msg_queue.intrpt.remote = rmt_intrpt;
		msg_queue.id = 1;
		msg_queue.nbr_msgs = 0;
		msg_queue.msg_read_offset = slotsize*nbr_slots;
		msg_queue.nbr_slots = nbr_slots;
		
		msg_out.push_back (msg_queue);
	}

	pthread_mutex_init (&sndrcv_lock, NULL);

	return;
}

SCI_msg::~SCI_msg ()
{
	sci_error_t sisci_err;

	// unlock remote memory and shutdown interrupts 
	for (vector< SCI_msgqueue_rmt >::iterator rmt_msgq_it = msg_out.begin();
		  macc_it != msg_out.end(); rmt_msgq_it++) {
		memacc.lock_release (rmt_msgq_it.base);
		SCIDisconnectInterrupt (rmt_msgq_it.rmt_intrpt, NULL, &sisci_err);
	}

	// release local ressources
	SCIRemoveInterrupt(local_intrpt, NULL, &sisci_err);
	memacc.lock_release (local_packets);

	pthread_mutex_destroy (&sndrcv_lock);
  
	return;
}

int SCI_msg::rank()
{
  return myrank;
}

int SCI_msg::size()
{
  return nbrprocs;
}

int SCI_msg::max_payload()
{
  return payload;
}

void SCI_msg::reg_msgtype (int *tag; (*handler)(int src, int tag, int lent, void *data), int flags)
{
	*tag = next_tag++;
	handlers[*tag] = handler;
  
	return;
}

void SCI_msg::free_msgtype (int tag)
{
	handlers[tag] = NULL;
  
	return;
}

void SCI_msg::send (int dst, int tag, int len, void *data)
{
	sci_error_t sisci_err;
	SCI_packet pkt;
	SCI_macc msg_macc;

	pthread_mutex_lock (&sndrcv_lock);
	
	ulong pkt_offset = get_pkt(dst);	

	// prepare & copy packet header, followed by the data
	pkt.id = msg_out[dst].id;
	msg_out[dst].id = ++msg_out[dst].id % SCI_MSG_MAXID;
	pkt.tag = tag;
	pkt.len = len;
	MACC_INIT(msg_macc, msg_out[dst].node_id, msg_out[dst].sgmt_id, msg_out[dst].next, 1);
	memacc.put (msg_macc, len, data);
	// signal the receiver
	SCITriggerInterrupt (msg_out[dst].intrpt.remote, NO_FLAGS, &sisci_err);
	
	pthread_mutex_unlock (&sndrcv_lock);
	return;
}

// poll until a matching message from one of the specified src ranks comes in
void SCI_msg::poll (vector<int>& src_ranks, vector<int>& src_tags, int *recv_tag, void *recv_data)
{
  
	return;
}

void SCI_msg::free_msg (void *buf)
{
	// make buffer available again by pushing buffer ptr to msg_buffers stack
	buffer_pool.push (buf);
	
	return;
}


//
// private functions
//

// get a pkt for a message to process 'dst' - returns an offset relative to 
// the base of the receive queue at 'dst'
ulong SCI_msg::get_pkt (int dst)
{
	static ulong offset;
	
	
	msg_out[dst].nbr_msgs++;
	
	return offset;
}

void *SCI_msg::get_msg_buf()
{
	void *buf;

	// try to pop one buffer ptr from the stack; if
	// no more buffers on the stack, allocate a new one
	if (!buffer_pool.empty()) {
		buf = buffer_pool.top();
		buffer_pool.pop();
	} else {
		buf = static_cast<void *>(new SCI_maxslot);
	}
	return buf;
}


sci_callback_action_t SCI_msg::msg_arrived (void *arg, sci_local_interrupt_t interrupt, sci_error_t status)
{
  static int queue_nbr = 0;
  void *msg_buf;

  pthread_mutex_lock (&sndrcv_lock);

  // check the incoming queues for new messages - drain all queues completely
  for (int nbr_checks = 0; nbr_checks < nbrprocs; nbr_checks++, ++queue_nbr % nbrprocs) {
	  while (msg_in[queue_nbr].id == msg_in[queue_nbr].next->id) {
		  // get the message & free the slot
		  msg_buf = get_msg_buf();
		  memcpy (msg_buf, &msg_in[queue_nbr].data, msg_in[queue_nbr].len);
		  *(msg_in[queue_nbr].local.msg_read)++;
		  
		  // call the appropiate msg handler
		  msg_handlers[msg_in[queue_nbr].tag](queue_nbr, msg_in[queue_nbr].tag, msg_in[queue_nbr].len, msg_buf);
		  
		  // update related data structures
		  msg_in[queue_nbr].id = ++msg_in[queue_nbr].id % SCI_MSG_MAXID;
		  msg_in[queue_nbr].next = (msg_in[queue_nbr].next == msg_in[queue_nbr].last) ?
			  msg_in[queue_nbr].base : static_cast<char *>(msg_in[queue_nbr].next) + slotsize;
	  }
  }

  pthread_mutex_unlock (&sndrcv_lock);
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
