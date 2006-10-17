/* $Id$ */

#ifndef _SMI_INTERNAL_FUNCTIONS_H_
#define _SMI_INTERNAL_FUNCTIONS_H_


#ifdef __cplusplus
extern "C" {
#endif
  
#ifndef _DYN_MEM_H_
  smi_error_t _smi_allocate_MMU_lock(int);
  smi_error_t _smi_address_to_MMUregion(char*, int*);
  smi_error_t SMI_Init_shregMMU(int);
  smi_error_t SMI_Imalloc(size_t, int, void**);
  smi_error_t SMI_Ifree(void*);
  smi_error_t SMI_Cfree(void*);
#endif
  
#ifndef SMEM_H
  int _smi_sinit (void * adresse, int max);
  void * _smi_salloc (void * adresse, unsigned int memsize);
  void _smi_sfree (void * adresse, void * speicher);
#endif
  
#ifndef _ERROR_COUNT_H_
  smi_error_t SMI_Check_transfer( int flags );
  smi_error_t SMI_Check_transfer_addr( void *address, int flags );
#ifndef NO_SISCI
  smi_error_t _smi_check_transfer( sci_sequence_t seq, int flags);
#endif
#endif
  
#ifndef _SMI_FINALIZE_H_
  smi_error_t SMI_Finalize(void);
#endif
  
#ifndef _INIT_H_
  smi_error_t SMI_Init(int*, char***);
#endif
  
#ifndef __FPU_COPY_H__
  void _smi_fpu_memcpy(char *dst, char *src, int n);
#endif
  
#ifndef __LOCAL_SEG_H
  smi_error_t _smi_map_local_segment(shseg_t* shseg);
  smi_error_t _smi_unmap_local_segment(shseg_t* shseg);
  smi_error_t _smi_create_local_segment(shseg_t* shseg);
#endif
  
#ifndef __SMI_MEMCPY_H
  int _smi_memcpy_init(void);
  int _smi_memcpy_finalize(void);
  smi_error_t SMI_Memcpy (void *dest, void *src, size_t size, int flags);
  smi_error_t SMI_Imemcpy (void *dest, void *src, size_t size, int flags, smi_memcpy_handle* h);
  smi_error_t SMI_Memwait (smi_memcpy_handle h);
  smi_error_t SMI_Memtest (smi_memcpy_handle h);
  smi_error_t SMI_MemwaitAll (int count, smi_memcpy_handle *h, smi_error_t *status); 
  smi_error_t SMI_MemtestAll (int count, smi_memcpy_handle *h, smi_error_t *status);
#endif
  
#ifndef __SCI_SHMEM_H
#ifndef NO_SISCI
  smi_error_t _smi_init_sci_subsystem(void);
  smi_error_t _smi_map_sci_shared_segment(shseg_t* shseg);
  smi_error_t _smi_unmap_sci_shared_segment(shseg_t* shseg);
  smi_error_t _smi_create_sci_shared_segment(shseg_t* shseg);
  smi_error_t _smi_remove_sci_shared_segment(shseg_t* shseg);
#endif 
#endif
  
#ifndef __SHMEM_H
  smi_error_t _smi_map_shared_segment(shseg_t* shseg);
  smi_error_t _smi_unmap_shared_segment(shseg_t* shseg);
  smi_error_t _smi_create_shared_segment(shseg_t* shseg);
  smi_error_t _smi_remove_shared_segment(shseg_t* shseg);
  smi_error_t _smi_init_shared_segment_subsystem(void);
#endif
    
#ifndef __SVM_SHMEM_H
  smi_error_t _smi_init_svm_subsystem(void);
  smi_error_t _smi_map_svm_shared_segment(shseg_t* shseg);
  smi_error_t _smi_unmap_svm_shared_segment(shseg_t* shseg);
  smi_error_t _smi_create_svm_shared_segment(shseg_t* shseg);
  smi_error_t _smi_remove_svm_shared_segment(shseg_t* shseg);
#endif
  
#ifndef __UNIX_SHMEM_H
  smi_error_t _smi_map_unix_shared_segment(shseg_t* shseg);  
  smi_error_t _smi_unmap_unix_shared_segment(shseg_t* shseg);
  smi_error_t _smi_create_unix_shared_segment(shseg_t* shseg);
  smi_error_t _smi_remove_unix_shared_segment(shseg_t* shseg);
#endif
  
#ifndef __SETUP_COMM_H
  smi_error_t _smi_build_reordered_communicator(void);
  smi_error_t _smi_build_machine_communicators(void);
#endif
  
#ifndef _PAGE_SIZE_H_
  smi_error_t SMI_Page_size(int*);
#endif
  
#ifndef _FIRST_PROC_ON_NODE_H_
  int _smi_first_proc_on_node(int);
  int _smi_last_proc_on_node(int );
#endif
  
  
#ifndef _NODE_RANK_H_
  smi_error_t SMI_Node_rank(int*);
#endif
  
#ifndef _NODE_SIZE_H_
  smi_error_t SMI_Node_size(int*);
#endif
  
#ifndef _SMI_PROC_RANK_H_
  int _smi_local_proc_rank (int proc);
  smi_error_t SMI_Proc_rank(int*);
  smi_error_t SMI_Local_proc_rank(int* rank);
#endif
  
#ifndef _SMI_PROC_SIZE_H_
  int _smi_procs_on_node (int node);
  smi_error_t SMI_Proc_size(int*);
  smi_error_t SMI_Local_proc_size(int* size);
  smi_error_t SMI_Max_local_proc_size(int* size);
#endif
  
#ifndef _PROC_TO_NODE_H_
  smi_error_t SMI_Proc_to_node(int, int*);
#endif
    
#ifndef __RESOURCE_LIST_H__
  int _smi_init_resource_list(void);
  int _smi_clear_all_resources(void); 
#ifndef NO_SISCI 
  void rs_SCIOpen(sci_desc_t   *sd,
		  unsigned int flags,
		  sci_error_t  *error);
  void rs_SCIClose(sci_desc_t sd,
		   unsigned int flags,
		   sci_error_t *error);
  void rs_SCICreateSegment(sci_desc_t             sd,
			   sci_local_segment_t    *segment,
			   unsigned int           segmentId,
			   size_t                 size,
			   sci_cb_local_segment_t callback,
			   void                   *callbackArg, 
			   unsigned int           flags,
			   sci_error_t            *error);
  void rs_SCIRemoveSegment(sci_local_segment_t segment,
			   unsigned int        flags, 
			   sci_error_t         *error);
  void rs_SCICreateMapSequence(sci_map_t   map, 
				 sci_sequence_t *sequence, 
				 unsigned int   flags, 
				 sci_error_t    *error);
  void rs_SCIRemoveSequence(sci_sequence_t sequence, 
			    unsigned int   flags, 
			    sci_error_t    *error);
  void rs_SCICreateDMAQueue(sci_desc_t      sd,
			    sci_dma_queue_t *dq,
			    unsigned int    localAdapterNo,
			    size_t          maxEntries,
			    unsigned int    flags,
			    sci_error_t     *error);
  void rs_SCIRemoveDMAQueue(sci_dma_queue_t dq,
			    unsigned int    flags,
			    sci_error_t     *error);
  void rs_SCICreateInterrupt(sci_desc_t            sd,
			     sci_local_interrupt_t *interrupt,
			     unsigned int          localAdapterNo,
			     unsigned int          *interruptNo,
			     sci_cb_interrupt_t    callback,
			     void                  *callbackArg,
			     unsigned int          flags,
			     sci_error_t           *error);
  void rs_SCIRemoveInterrupt( sci_local_interrupt_t interrupt,
			      unsigned int          flags,
			      sci_error_t           *error);
  void rs_SCIConnectInterrupt(sci_desc_t            sd,
			      sci_remote_interrupt_t *interrupt,
			      unsigned int           nodeId,
			      unsigned int           localAdapterNo,
			      unsigned int           interruptNo,
			      unsigned int           timeout,
			      unsigned int           flags,
			      sci_error_t            *error);
  void rs_SCIDisconnectInterrupt(sci_remote_interrupt_t interrupt,
				 unsigned int          flags,
				 sci_error_t           *error);
  void rs_SCIConnectSegment( sci_desc_t sd,
			     sci_remote_segment_t* segment,
			     unsigned int nodeId,
			     unsigned int segmentId,
			     unsigned int localAdapterNo,
			     sci_cb_remote_segment_t callback,
			     void* callbackArg,
			     unsigned int timeout,
			     unsigned int flags,
			     sci_error_t* error);
  void rs_SCIDisconnectSegment( sci_remote_segment_t segment,
				unsigned int flags,
				sci_error_t* error);
#endif
  int rs_shmget(key_t key, size_t size, int shmflg);
  int rs_shmctl(int shmid, int cmd, struct shmid_ds *buf);
  int rs_CreateTempfile(const char* szName, int oflag);
  void rs_RemoveTempfile(char* szName);
  FILE *rs_CreateTempstream(const char* szName, const char *mode);
  void rs_RemoveTempstream(char* szName);
#endif
  
#ifndef __WATCHDOG_H__
  smi_error_t SMI_Watchdog(int iWatchdogTimeout);
  int _smi_wd_set_timeout(int iWatchdogTimeout);
  void SMI_Abort(int a);
  void _smi_init_signal_handler(void);
  void _smi_init_watchdog(int ThisP, int NrOfP, int volatile *BaseAddr);
  void _smi_finalize_watchdog(void);
  int _smi_wd_set_timeout(int iWatchdogTimeout);
  void _smi_wd_enable(int);
  void _smi_wd_disable(void);
#endif
  
#ifndef _REDIRECT_IO_H_
  smi_error_t SMI_Redirect_IO(int err, void* errparam, 
			  int out, void* outparam, 
			  int in, void* inparam);
  void _smi_close_redirected_io(void);
#endif
  
#ifndef _SMI_ADDRESS_TO_REGION_H_
  smi_error_t SMI_Adr_to_region(void* address, int* region_id);
  smi_error_t _smi_shseg_to_region(shseg_t* shseg, int* region_id);
  shseg_t *_smi_addr_to_shseg(void *addr);
#endif
  
#ifndef _SMI_CREATE_SHREG_H_
  smi_error_t _smi_prepare_for_new_region(int*, region_t**);
  smi_error_t _smi_create_region_data_structure(int, smi_region_info_t*, int*, char**, region_t**, device_t, unsigned int);
  smi_error_t SMI_Create_shreg(int, smi_region_info_t*, int*, void**);
#endif
  
#ifndef _FREE_SHREG_H_
  smi_error_t SMI_Free_shreg(int);
  int _smi_free_shreg(int);
#endif
  
#ifndef _PRINT_REGIONS_H_
  void _smi_print_segment(shseg_t*);
  void _smi_print_regions(void);
#endif
  
#ifndef _REGION_LAYOUT_H_
  smi_error_t SMI_Region_layout(int region_id, smi_rlayout_t** r);
#endif
  
#ifndef _SEGMENT_ADDRESS_H_
  smi_error_t _smi_determine_start_address(char** address, size_t size);
#endif
  
#ifndef __SETUP_H
  smi_error_t _smi_get_no_processes(void);
  smi_error_t _smi_get_loc_mpi_rank(void);
  smi_error_t _smi_get_page_size(void);
  smi_error_t _smi_set_ranks(void);
  smi_error_t _smi_set_no_machines(void);
  smi_error_t _smi_determine_closeness(void);
#endif
    
#ifndef __SMI_FILESYNC_H__
  int _smi_FileBroadcast(char* szFileName, void* pData, size_t stLen, int iMyId, int iRoot, int iNumProcs);
#endif
  
#ifndef __COMBINE_ADD_H
  smi_error_t _smi_combine_add(region_t* shared, region_t* local, int param1,
			   int param2, int comb_mode);
#endif
    
#ifndef _COPY_H_
  smi_error_t _smi_copy_from_to(void*, void*, size_t, int);
  smi_error_t _smi_copy_from_to_double(void*, void*, size_t, int);
#endif
  
#ifndef __COPY_EVERY_LOCAL_H
  smi_error_t _smi_copy_every_local(int global_region_id, int local_region_id);
#endif
    
#ifndef __COPY_GL_DIST_LOCAL_H
  smi_error_t _smi_copy_globally_distributed_to_local(int global_region_id, int local_region_id);
#endif
  
#ifndef _ENSURE_CONSISTENCY_H_
#define _ENSURE_CONSISTENCY_H_
  smi_error_t SMI_Ensure_consistency(int, int, int, int);
#endif
    
#ifndef __INIT_SWITCHING_H
#define __INIT_SWITCHING_H
  extern int* loop_bound_array;
  int _smi_init_switching(void);
#endif
  
#ifndef _SWITCH_TO_REPLICATION_H_
  smi_error_t _smi_id_to_region_struct(int, region_t**);
  smi_error_t SMI_Switch_to_replication(int, int, int, int, int);
#endif
  
#ifndef _SWITCH_TO_REPLICATION_FAST_H_
    smi_error_t SMI_Switch_to_replication_fast(int, int, int, int, int, void**);
#endif
  
#ifndef _SWITCH_TO_SHARING_H_
  smi_error_t _smi_combine_add_double(region_t*, region_t*, int, int, int);
  smi_error_t _smi_combine_loop_splitting(int, int, region_t*, region_t*);
  smi_error_t SMI_Switch_to_sharing(int, int, int, int);
#endif
    
#ifndef _SWITCH_TO_SHARING_FAST_H_
  smi_error_t SMI_Switch_to_sharing_fast(int, int, int, int, void**);
#endif
  
#ifndef _SMI_MUTEX_H_
  smi_error_t SMI_Mutex_lock(int id);
  smi_error_t SMI_Mutex_unlock(int id);
  void _smi_mutex_module_init(void);
  void _smi_mutex_module_finalize(void);
  smi_error_t SMI_MUTEX_INIT(int* id, int dummy_alorithm_type, int dummy_prank);
  smi_error_t SMI_Mutex_destroy(int);
  smi_error_t SMI_Mutex_trylock(int, int*);
#endif
    
#ifndef _BARRIER_H_
  smi_error_t SMI_BARRIER_INIT(int*, int, int);
  smi_error_t SMI_BARRIER_DESTROY(int);
  smi_error_t SMI_BARRIER(int);
#endif
  
#ifndef _PROGRESS_H_
  smi_error_t SMI_Init_PC(int* id);
  smi_error_t SMI_Reset_PC(int id);
  smi_error_t SMI_Increment_PC(int id, int val);
  smi_error_t SMI_Get_PC(int pcid, int proc_id, int* pc_val);
  smi_error_t SMI_Wait_individual_PC(int id, int proc_rank, int val);
  smi_error_t SMI_Wait_collective_PC(int id, int val);
#endif
  
#ifndef __SIGNALIZATION_H__
  smi_error_t SMI_Signal_wait (int proc_rank);
  smi_error_t SMI_Signal_setCallBack (int proc_rank, void (*callback_fcn)(void *), 
				  void *callback_arg, smi_signal_handle* h);
  smi_error_t SMI_Signal_joinCallBack (smi_signal_handle* h);
  smi_error_t SMI_Signal_send ( int proc_rank );
  int _smi_signal_init(int, int);
  int _smi_signal_finalize(void);
#endif
  
#ifndef __SENDRECV_H__
smi_error_t _smi_init_mp(void);
smi_error_t _smi_finalize_mp(void);

smi_error_t SMI_Send(void *buf, int count, int dest);
smi_error_t SMI_Recv(void *buf, int count, int dest);

smi_error_t SMI_Isend(void *buf, int count, int dest);
smi_error_t SMI_Send_wait(int dest);

smi_error_t SMI_Sendrecv(void *send_buf, void *recv_buf, int count, int dest);
#endif


#ifndef _STORE_BARRIER_H_
  void _smi_init_load_store_barriers();
  void _smi_store_barrier();
  void _smi_local_store_barrier();
  int _smi_load_barrier();
  int _smi_range_store_barrier(volatile void* start, unsigned int size, int home_of_data);
  int _smi_range_load_barrier(volatile void* start, unsigned int size, int home_of_data);
#endif
  
#ifndef _SMI_SYNC_FINALIZE_H_
  smi_error_t _smi_synchronization_finalize(void);
#endif
  
#ifndef _SMI_SYNC_INIT_H_
  smi_error_t _smi_synchronization_init(int use_signals);
#endif
  
#ifndef _GENERAL_H_
  int imin(int, int);
  int imax(int, int);
  void _smi_intersection(int, int, int, int, int*, int*);
  int _smi_local_rank(int);
  int _smi_no_processes_on_machine(int);
#endif
  
#ifndef _SMI_QUERY_H_
  smi_error_t SMI_Query (smi_query_t cmd, int arg, void *result);
  smi_error_t _smi_init_query (int nbr_procs);
#endif
    
#ifndef _SMI_TIME_H_
  double SMI_Wtime(void);
  void SMI_Get_ticks(void *ticks);
  smi_error_t SMI_Get_timer(int* sec, int* microsec);
  smi_error_t SMI_Get_timespan(int* sec, int* microsec);
  void _smi_init_timer(void);
#endif
  
    
#ifdef __cplusplus
}
#endif


#endif
