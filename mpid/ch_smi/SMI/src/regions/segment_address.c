/* $Id$ */

#include "segment_address.h"
#include "env/smidebug.h"
#include "create_shreg.h"
#include "memory/shmem.h"
#include "message_passing/lowlevelmp.h"

#ifndef WIN32

#include <sys/resource.h> 

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/*****************************************************************************/
/*** Internal function: determine a free portion of the virtual address    ***/
/*** space that is 'size' byte large.                                      ***/
/*****************************************************************************/
static smi_error_t _smi_determine_start_address_unix(char** address, size_t size)
{
  REMDSECTION("_smi_determine_start_address_unix");
 
  struct rlimit rlp;
  extern _etext, _edata, _end; 
  int i, j;
  boolean fit;
  boolean fixed_region_exists = false;
  char *round_addr;
    
  DSECTENTRYPOINT;
  
  SMI_LOCK(&_smi_mis_lock);

  /* is this the first (valid) region that is to be mapped fixed ? */
  for (i = 0; i < _smi_mis.no_regions; i++)
      if ((_smi_mis.region[i]->id >= 0) &&
	  !(_smi_mis.region[i]->seg[0]->flags & SHREG_NONFIXED))
	  fixed_region_exists = true;
  
  if (!fixed_region_exists) {
    /* this is the first fixed region to be _smi_allocated; start */
    /*  in the middle of the address space                        */
#ifdef LINUX
    /* XXX try to do it dynamically */
    *address = (char*)(0x04000000 + 256 * 1024);
#endif
#ifdef SPARC
    /* XXX try to do it dynamically 
     *address = (char*)0x60000000; */
#endif
    
#if defined SOLARIS
    /* dynamically determine a good address to map segments to */
    /* XXX usable for SOLARIS_SPARC, too? */
    
    /* not needed yet - could be used to determine upper limit of address
       space for the shared segments */
    if(getrlimit(RLIMIT_STACK, &rlp))
      perror("getrlimit()");
    
    if (getrlimit(RLIMIT_DATA, &rlp))
      perror("getrlimit()");
    
    round_addr = (char*)(((ulong)&_etext) + rlp.rlim_max);
    *address = (char *)((int)round_addr & 0xf0000000);
    round_addr = (char *)(((int)round_addr & 0x0f000000) / 0x01000000);
    *address = (char *)((int)*address + ((int)(round_addr + 1) * 0x01000000));
    DNOTICEP("_etext + RLIMIT_DATA == ", ((ulong)&_etext) + rlp.rlim_max);  
    DNOTICEP("base addr for shared segments == ", *address);
#endif
  } else {
      /* find a valid address between the existing segments 
	 XXX this is not safe if the user does mmap()s himself! */
      unsigned long addr_j, size_j, addr_new;
      
      fit = false;
      i = 0;
      while ((fit == false) && (i < _smi_mis.no_regions)) {
	  /* we do not consider the addresses of non-fixed regions since
	     this leads to invalid addresses */
	  while ((i < _smi_mis.no_regions) && (_smi_mis.region[i]->seg[0]->flags & SHREG_NONFIXED))
	      i++;
	  if (i == _smi_mis.no_regions)
	      break;
	  addr_new = (unsigned long)((unsigned long)_smi_mis.region[i]->addresses[0] 
				    + _smi_mis.region[i]->size);
	  
	  /* check, if this address does fit: the new segment must not be reach into the 
	     address range of region 'j' */
	  fit = true;
	  for (j = 0; j < _smi_mis.no_regions; j++) {
	      addr_j = (unsigned long)_smi_mis.region[j]->addresses[0];
	      size_j =  _smi_mis.region[j]->size;
	      if ((addr_j >= addr_new && addr_j <= addr_new + size-1)
		  || (addr_j + size_j - 1 >= addr_new && addr_j + size_j - 1 <= addr_new + size - 1))
		  fit = false;
	  }
	  i++;
      }
      *address = (char *)addr_new;
  }
  
  SMI_UNLOCK(&_smi_mis_lock);

  DSECTLEAVE;
  return(SMI_SUCCESS);
}

#else /* WIN32 */

static char *address_counter = (char *) VIRTUAL_MEM_MAX;

/*****************************************************************************/
/*** Internal function: determine a free portion of the virtual address    ***/
/*** space that is 'size' byte large.                                      ***/
/*****************************************************************************/
smi_error_t _smi_determine_start_address_win32(char** address, size_t size)
 {
   DSECTION("_smi_determine_start_address_win32");
   BOOL fOk, IwasHere=FALSE;
   MEMORY_BASIC_INFORMATION mem_info;
   SYSTEM_INFO sys_info;
   smi_error_t mpi_error;
   smi_error_t error;
   int i;
   char **address_array;
   size_t *size_array;
   
   DSECTENTRYPOINT;

   ALLOCATE( size_array, size_t*, (sizeof(int) * _smi_nbr_procs));
   mpi_error = _smi_ll_allgather((int*) &size,sizeof(size_t)/sizeof(int),(int*) size_array,_smi_my_proc_rank);
   ASSERT_R((mpi_error == MPI_SUCCESS),"_smi_ll_allgather failed",1000+mpi_error);
   for(i=1,size=size_array[0];i<_smi_nbr_procs;i++)
     if(size<size_array[i])
       size=size_array[i];
   free(size_array);

   /* round size to the next multiple of the     */
   /* smallest granularity that can be _smi_allocated */
   GetSystemInfo(&sys_info);
   if (size % sys_info.dwAllocationGranularity != 0)
     size += (sys_info.dwAllocationGranularity - (size % sys_info.dwAllocationGranularity));
   
   DNOTICE("Allocating memory for address array");
   ALLOCATE( address_array, char **, sizeof(char *) * _smi_nbr_procs );
   ASSERT_R((address_array!=NULL),"Could not get memory for adress_array",2000+errno);

   do {  
     DNOTICE("Getting local address available");
     do {
       address_counter -= size;  	 
       if (address_counter < (char *) VIRTUAL_MEM_MIN)
	 break;
       VirtualQuery(address_counter, &mem_info, sizeof(mem_info));
       if (mem_info.State != MEM_FREE)
	 address_counter = mem_info.AllocationBase;
     } while ((mem_info.State != MEM_FREE) || (mem_info.RegionSize < size));

     DNOTICEP("Got address",address_counter);
     address_array[_smi_my_proc_rank] = address_counter;
  
     DNOTICE("Gathering all addresses");
     mpi_error = _smi_ll_allgather((int*)&address_counter, sizeof(char*)/sizeof(int), 
				   (int*)address_array, _smi_my_proc_rank);
     ASSERT_R((mpi_error == MPI_SUCCESS),"_smi_ll_allgather failed",1000+mpi_error);
     
     fOk = TRUE;
     address_counter = address_array[0];
     
     for(i=1; i<_smi_nbr_procs; i++)
     {
       if (address_counter < (char *) VIRTUAL_MEM_MIN)
       {
	 DNOTICE("adress_counter is less VIRTUAL_MEM_MIN");
	 fOk = FALSE;
	 address_counter = (char *) VIRTUAL_MEM_MAX;
	 if (IwasHere == TRUE)
	 {	
	   free(address_array);
	   
	   DPROBLEM("This code-part shall only be executed once");
	   DSECTLEAVE
	     return(SMI_ERR_OTHER);
	 } else
	   IwasHere = TRUE;
	 break;
       }
       if (address_counter > address_array[i])
       {
	 DNOTICEI("address_counter is greater address_array",i);
	 fOk = FALSE;
	 DNOTICEP("New address taken:", address_array[i]);
	 address_counter = address_array[i];
       } 
       else 
	 if  (address_counter < address_array[i])
	   fOk = FALSE;
     }
   } while(fOk == FALSE);
   
   DNOTICE("Freeing memory for address array");
   free(address_array);
   address_counter += size;
#ifdef WINDOWS95
   *address = mem_info.BaseAddress;
#else
   *address = VirtualAlloc(mem_info.BaseAddress, size, MEM_RESERVE, PAGE_READWRITE|PAGE_NOCACHE);
   if (*address == NULL)
   {
     error = GetLastError();
     DPROBLEM("VirtualAlloc failed");
     DSECTLEAVE
       return (2000+error);
   }
#endif
   
   DSECTLEAVE
     return(SMI_SUCCESS);
 }

#endif /* WIN32 */

smi_error_t _smi_determine_start_address(char** address, size_t size)
{
#ifdef WIN32
  return(_smi_determine_start_address_win32(address,size));
#else 
  return(_smi_determine_start_address_unix(address,size));
#endif
}
