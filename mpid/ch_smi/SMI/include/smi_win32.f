CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     This file contains all definitions of constants necessary in
C     conjunction with the SMI-library.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

C	Interfaces to the SMI functions
	INTERFACE  
		
	SUBROUTINE smi_init(error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_init_' :: SMI_INIT
	INTEGER error
	END SUBROUTINE smi_init

	SUBROUTINE smi_finalize(error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_finalize_' :: SMI_FINALIZE
	INTEGER error
	END SUBROUTINE smi_finalize

	SUBROUTINE smi_proc_rank(rank, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_proc_rank_' :: SMI_PROC_RANK
	INTEGER rank, error
	END SUBROUTINE smi_proc_rank

	SUBROUTINE smi_proc_size(size, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_proc_size_' :: SMI_PROC_SIZE
	INTEGER size, error
	END SUBROUTINE smi_proc_size

	SUBROUTINE smi_node_rank(rank, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_node_rank_' :: SMI_NODE_RANK
	INTEGER rank, error
	END SUBROUTINE smi_node_rank

	SUBROUTINE smi_node_size(size, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_node_size_' :: SMI_NODE_SIZE
	INTEGER size, error
	END SUBROUTINE smi_node_size

	SUBROUTINE smi_proc_to_node(proc, node, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_proc_to_node_' :: SMI_PROC_TO_NODE
	INTEGER proc, node, error
	END SUBROUTINE smi_proc_to_node
	
	SUBROUTINE smi_page_size(pz, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_page_size_' :: SMI_PAGE_SIZE
	INTEGER pz, error
	END SUBROUTINE smi_page_size

	SUBROUTINE smi_create_shreg(size, dist_policy, dist_values, id, address, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_create_shreg_' :: SMI_CREATE_SHREG
	INTEGER size, dist_policy, dist_values, id
	INTEGER*4 address
	INTEGER error
	END SUBROUTINE smi_create_shreg

	SUBROUTINE smi_free_shreg(id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_free_shreg_' :: SMI_FREE_SHREG
	INTEGER id, error
	END SUBROUTINE smi_free_shreg

	SUBROUTINE smi_adr_to_region(adr, region_id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_adr_to_region_' :: SMI_ADR_TO_REGION
	INTEGER*4 adr
	INTEGER region_id, error
	END SUBROUTINE smi_adr_to_region

	SUBROUTINE smi_switch_to_replication(id, mode, p1, p2, p3, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_switch_to_replication_' :: SMI_SWITCH_TO_REPLICATION
	INTEGER id, mode, p1, p2, p3, error
	END SUBROUTINE smi_switch_to_replication

	SUBROUTINE smi_switch_to_sharing(id, comb_mode, p1, p2, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_switch_to_sharing_' :: SMI_SWITCH_TO_SHARING
	INTEGER id, comb_mode, p1, p2, error
	END SUBROUTINE smi_switch_to_sharing

	SUBROUTINE smi_ensure_consistency(id, comb_mode, p1, p2, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_ensure_consistency_' :: SMI_ENSURE_CONSISTENCY
	INTEGER id, comb_mode, p1, p2, error
	END SUBROUTINE smi_ensure_consistency

	SUBROUTINE smi_mutex_init(id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_mutex_init_' :: SMI_MUTEX_INIT
	INTEGER id, error
	END SUBROUTINE smi_mutex_init

    	SUBROUTINE smi_mutex_init_with_locality(id, proc, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_mutex_init_with_locality_' :: SMI_MUTEX_INIT_WITH_LOCALITY
	INTEGER id, proc, error
	END SUBROUTINE smi_mutex_init_with_locality

	SUBROUTINE smi_mutex_lock(id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_mutex_lock_' :: SMI_MUTEX_LOCK
	INTEGER id, error
	END SUBROUTINE smi_mutex_lock

	SUBROUTINE smi_mutex_unlock(id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_mutex_unlock_' :: SMI_MUTEX_UNLOCK
	INTEGER id, error
	END SUBROUTINE smi_mutex_unlock

	SUBROUTINE smi_mutex_destroy(id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_mutex_destroy_' :: SMI_MUTEX_DESTROY
	INTEGER id, error
	END SUBROUTINE smi_mutex_destroy

	SUBROUTINE smi_barrier(error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_barrier_' :: SMI_BARRIER
	INTEGER error
	END SUBROUTINE smi_barrier

	SUBROUTINE smi_init_shregmmu(region_id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_init_shregmmu_' :: SMI_INIT_SHREGMMU
	INTEGER region_id, error
	END SUBROUTINE smi_init_shregmmu

	SUBROUTINE smi_imalloc(size, region_id, address, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_imalloc_' :: SMI_IMALLOC
	INTEGER size, region_id
	INTEGER*4 address
	INTEGER error
	END SUBROUTINE smi_imalloc

	SUBROUTINE smi_cmalloc(size, region_id, address, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_cmalloc_' :: SMI_CMALLOC
	INTEGER size, region_id
	INTEGER*4 address
	INTEGER error
	END SUBROUTINE smi_cmalloc

	SUBROUTINE smi_ifree(address, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_ifree_' :: SMI_IFREE
	INTEGER*4 address
	INTEGER error
	END SUBROUTINE smi_ifree

	SUBROUTINE smi_cfree(address, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_cfree_' :: SMI_CFREE
	INTEGER*4 address
	INTEGER error
	END SUBROUTINE smi_cfree

	SUBROUTINE smi_get_timer(sec, microsec, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_get_timer_' :: SMI_GET_TIMER
	INTEGER sec, microsec,error
	END SUBROUTINE smi_get_timer

	SUBROUTINE smi_get_timespan(sec, microsec, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_get_timespan_' :: SMI_GET_TIMESPAN
	INTEGER sec, microsec,error
	END SUBROUTINE smi_get_timespan

	SUBROUTINE smi_loop_init(id, low, high, mode, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_loop_init_' :: SMI_LOOP_INIT
	INTEGER id, low, high, mode, error
	END SUBROUTINE smi_loop_init

	SUBROUTINE smi_get_iterations(id, status, low, high, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_get_iterations_' :: SMI_GET_ITERATIONS
	INTEGER id, status, low, high, error
	END SUBROUTINE smi_get_iterations

	SUBROUTINE smi_loop_free(id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_loop_free_' :: SMI_LOOP_FREE
	INTEGER id, error
	END SUBROUTINE smi_loop_free

	SUBROUTINE smi_evaluate_speed(speedarray, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_evaluate_speed_' :: SMI_EVALUATE_SPEED
	REAL*8 speedarray
	INTEGER error
	END SUBROUTINE smi_evaluate_speed

	SUBROUTINE smi_use_evaluated_speed(id, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_use_evaluated_speed_' :: SMI_USE_EVALUATED_SPEED
	INTEGER id, error
	END SUBROUTINE smi_use_evaluated_speed

	SUBROUTINE smi_set_loop_param(id, k, minl, minr, maxl, maxr, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_set_loop_param_' :: SMI_SET_LOOP_PARAM
	INTEGER id
	REAL*8 k
	INTEGER minl, minr, maxl, maxr, error
	END SUBROUTINE smi_set_loop_param

	SUBROUTINE smi_set_loop_help_param(id, helpdist, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_set_loop_help_param_' :: SMI_SET_LOOP_HELP_PARAM
	INTEGER id, helpdist, error
	END SUBROUTINE smi_set_loop_help_param

	SUBROUTINE smi_loop_k_adaption_mode(id, adapmode, maxcalcdist, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_loop_k_adaption_mode_' :: SMI_LOOP_K_ADAPTION_MODE
	INTEGER id, adapmode, maxcalcdist, error
	END SUBROUTINE smi_loop_k_adaption_mode

	SUBROUTINE smi_redirect_io(err, errparam, out, outparam, in, inparam, error)
	!MS$ATTRIBUTES C, REFERENCE, ALIAS:'_smi_redirect_io_' :: SMI_REDIRECT_IO
	INTEGER err
	CHARACTER*(*) errparam
	INTEGER out
	CHARACTER*(*) outparam
	INTEGER in
	CHARACTER*(*) inparam
	INTEGER error
	END SUBROUTINE smi_redirect_io

	END INTERFACE



C     Shared memory region's physical distribution policy 
     
      INTEGER UNDIVIDED, BLOCKED, CUSTOMIZED
      PARAMETER (UNDIVIDED = 0, BLOCKED = 1, CUSTOMIZED = 2)


C     Error codes

      INTEGER SMISUCCESS, SMIERROTHER, SMIERRNOINIT, SMIERRPARAM 
      INTEGER SMIERRBADADR, SMIERRMAPFAILED, SMIERRNODEVICE
      INTEGER SMIERRNOSEGMENT, SMIERRNOMEM
      PARAMETER (SMISUCCESS = 0,  SMIERROTHER = 1,  SMIERRNOINIT = 2)
      PARAMETER (SMIERRPARAM = 3, SMIERRBADADR = 4, SMIERRMAPFAILED = 5)
      PARAMETER (SMIERRNODEVICE = 6, SMIERRNOSEGMENT = 7)
      PARAMETER (SMIERRNOMEM = 8)


C     Combination/Replication strategies

      INTEGER SINGLESOURCE, LOOPSPLITTING, ADD, NOTHING
      INTEGER EVERYTHING, LOCALANDBEYOND, SPARSE, BAND, ONEPERNODE
      INTEGER FIXPOINT, FLOATINGPOINT, HIGHPRECISION, EVERYLOCAL 
      PARAMETER (SINGLESOURCE = 1, LOOPSPLITTING = 2, ADD = 4)
      PARAMETER (NOTHING = 8, EVERYTHING = 16, LOCALANDBEYOND = 32)
      PARAMETER (EVERYLOCAL = 64)
      PARAMETER (SPARSE = 256, BAND = 512, ONEPERNODE = 1024)
      PARAMETER (FIXPOINT = 2048, FLOATINGPOINT = 4096)
      PARAMETER (HIGHPRECISION = 8192)

C     Loop Scheduling
	
	INTEGER SMIPARTBLOCKED, SMIPARTCYCLIC, SMIPARTADAPTEDBLOCKED
	INTEGER SMIPARTTIMEDBLOCKED, SMILOOPREADY, SMILOOPLOCAL
	INTEGER SMILOOPREMOTE, SMINOADAPT, SMIADAPTEXPO, SMIADAPTLINEAR
	INTEGER SMIADAPTOPT, SMIHELPONLYSMP, SMINOCHANGE

      PARAMETER (SMIPARTBLOCKED        = 1)
      PARAMETER (SMIPARTCYCLIC         = 2)
      PARAMETER (SMIPARTADAPTEDBLOCKED = 3)
      PARAMETER (SMIPARTTIMEDBLOCKED   = 4)
      PARAMETER (SMILOOPREADY          = 0)
      PARAMETER (SMILOOPLOCAL          = 1)
      PARAMETER (SMILOOPREMOTE         = 2)
      PARAMETER (SMINOADAPT            = 1)
      PARAMETER (SMIADAPTEXPO          = 2)
      PARAMETER (SMIADAPTLINEAR        = 3)
      PARAMETER (SMIADAPTOPT           = 4)
      PARAMETER (SMIHELPONLYSMP        = -1)
      PARAMETER (SMINOCHANGE           = 0)

C	I/O Redirection

	INTEGER SMIIOASIS, SMIIOFILE

	PARAMETER (SMIIOASIS     = 0)
	PARAMETER (SMIIOFILE     = 1)
