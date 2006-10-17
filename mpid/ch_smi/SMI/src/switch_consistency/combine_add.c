/* $Id$ */

#include "env/general_definitions.h"
#include "combine_add.h"
#include "init_switching.h"
#include "synchronization/barrier.h"
#include "dyn_mem/dyn_mem.h" 
#include "env/smidebug.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif
  

/********************************************************************************/
/* This is for the sparse case. This function only requires few and constant- ***/
/*** size further shared memory in internal regions.                          ***/
/********************************************************************************/
smi_error_t _smi_combine_add_sparse_small(region_t* shared, region_t* local, int param1,
				 int param2, int comb_mode)
{	
  	smi_error_t error;
	int i, j; 
        int k;			/* Auxiliary counter var */
        int remote;		/* Rank of corresponding process when transfering */
        int block_id;		/* Block in remote cache to put data from local */
        int no_values; 		/* No of valid entries inside caching area */
        int scale=param1/8192;	/* No of transposed scans */
        int proc_val		/* No of local values */
        =param1/_smi_nbr_procs;
        int s_proc_val		/* Scaled no of local processed values */
        =param1/(_smi_nbr_procs*scale);
        int cache_area		/* Size of one cache area */
        =s_proc_val+1;
        int jump		/* Distance between consecutive areas */
        =(scale-1)*s_proc_val;
        int cache_entries	/* Scaled cache entry size */
        =(_smi_nbr_procs-1)*cache_area;
	int my_bottom		/* Lower limit of my cache window */
        =_smi_my_proc_rank*param1/_smi_nbr_procs;
	int my_top		/* Upper limit of my cache window */
        =(_smi_my_proc_rank+1)*param1/_smi_nbr_procs;        
        int offset;		/* Relative R/W position in current cache block */

   	/* Declare struct for caching different vector data types */
	struct i_cache
	{	int position;
		int value;
	};
	struct l_cache
	{	int position;
		long int value;
	};
	struct f_cache
	{	int position;
		float value;
	};
	struct d_cache
	{	int position;
		double value;
        };

   	/* Some casts and cache vars */        
        int* ilocal		= (int*)local->addresses[0];
        int* ishared		= (int*)shared->addresses[0];
        struct i_cache** icache;

        long int* llocal	= (long int*)local->addresses[0];
        long int* lshared	= (long int*)shared->addresses[0];
        struct l_cache** lcache;

        float* flocal		= (float*)local->addresses[0];
        float* fshared		= (float*)shared->addresses[0];
        struct f_cache** fcache;

        double* dlocal		= (double*)local->addresses[0];
        double* dshared		= (double*)shared->addresses[0];
        struct d_cache** dcache;

   	/* Arrays for pointers to different cache areas */
        ALLOCATE( icache, struct i_cache**, _smi_nbr_procs * sizeof(struct i_cache*) );
        ALLOCATE( lcache, struct l_cache**, _smi_nbr_procs * sizeof(struct l_cache*) );
        ALLOCATE( fcache, struct f_cache**, _smi_nbr_procs * sizeof(struct f_cache*) );
        ALLOCATE( dcache, struct d_cache**, _smi_nbr_procs * sizeof(struct d_cache*) );
        
	/* Initializing shared region type depending */
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) ishared[i]=0;
           	for(i=0; i < _smi_nbr_procs; i++)
                   	error=SMI_Cmalloc(sizeof(struct i_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(icache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_small proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
        }
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) lshared[i]=0;
           	for(i=0; i < _smi_nbr_procs; i++)
                   	error=SMI_Cmalloc(sizeof(struct l_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(lcache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_small proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) fshared[i]=(float)0.0;
           	for(i=0; i < _smi_nbr_procs; i++)
                   	error=SMI_Cmalloc(sizeof(struct f_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(fcache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_small proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
        {	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) dshared[i]=0.0;
                for(i=0; i < _smi_nbr_procs; i++)
                   	error=SMI_Cmalloc(sizeof(struct d_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(dcache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_small proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
	}
        
	SMI_Barrier();

/* This routine is especially usefull when adding sparse vectors. Every process */
/* scans his own vector to find valid elements. If these are in his own shared 	*/
/* mem area they are added immediately, otherwise they are transfered via SCI to*/
/* corresponding remote process. In this case just the element together with 	*/
/* his local position is copied. A barrier follows to close up local operation. */ 
/* During next cycle the transfered data is elementwise locally added and 	*/
/* finally parts are added to shared area.					*/
/* In contrast to SINGLE.C this version allows multiple runs to save mem in case*/
/* cache areas become too large							*/

	if((comb_mode & SMI_DTYPE_FIXPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
        {  /* No of transposed scans */
           for(k=0; k < scale; k++)
           {  	/* Init cache again before each sequence */
                for(i=0; i < cache_entries; i+=cache_area)
                   	icache[_smi_my_proc_rank][i].position=0;
                SMI_Barrier();
                /* Reset start base address */
                i=0;
                /* Scan to the end of shared vector */
                while(i+k*s_proc_val < param1)
                {	/* Check weather data is valid */
                        if(ilocal[i+k*s_proc_val])
                        {	/* ... and if it's local */
                                if((i+k*s_proc_val < my_bottom) 
                                || (i+k*s_proc_val > my_top))
                                {	/* ID of remote process */
                                        remote=(i+k*s_proc_val)/proc_val;
                                        /* Block id in remote cache is determined */
                                        /* from distance between blocks */
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
                                        icache[remote][cache_area*block_id].position++;
                                        offset
                                        =icache[remote][cache_area*block_id].position;
                                        icache[remote][cache_area*block_id+offset]                                              .position=i+k*s_proc_val;
                                        icache[remote][cache_area*block_id+offset]
                                        .value=ilocal[i+k*s_proc_val];
                                }
                                else
                                   	ishared[i+k*s_proc_val]+=ilocal[i+k*s_proc_val];
                        }
                        i++;
                        if(!(i%s_proc_val)) i+=jump;
                }
                SMI_Barrier();
                /* Adding the cached elements to shared vector */
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=icache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	ishared[icache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=icache[_smi_my_proc_rank][i*cache_area+j].value;
                }
           }
           for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(icache[i]));
        }
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
        {  for(k=0; k < scale; k++)
           {  	for(i=0; i < cache_entries; i+=cache_area)
                   	lcache[_smi_my_proc_rank][i].position=0;
                SMI_Barrier();
                i=0;
                while(i+k*s_proc_val < param1)
                {	if(llocal[i+k*s_proc_val])
                        {	if((i+k*s_proc_val < my_bottom) 
                                || (i+k*s_proc_val > my_top))
                                {	remote=(i+k*s_proc_val)/proc_val;
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
                                        lcache[remote][cache_area*block_id].position++;
                                        offset
                                        =lcache[remote][cache_area*block_id].position;
                                        lcache[remote][cache_area*block_id+offset]                                              .position=i+k*s_proc_val;
                                        lcache[remote][cache_area*block_id+offset]
                                        .value=llocal[i+k*s_proc_val];
                                }
                                else
                                   	lshared[i+k*s_proc_val]+=llocal[i+k*s_proc_val];
                        }
                        i++;
                        if(!(i%s_proc_val)) i+=jump;
                }
                SMI_Barrier();
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=lcache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	lshared[lcache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=lcache[_smi_my_proc_rank][i*cache_area+j].value;
                }
           }
           for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(lcache[i]));
        }
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
        {  for(k=0; k < scale; k++)
           {  	for(i=0; i < cache_entries; i+=cache_area)
                   	fcache[_smi_my_proc_rank][i].position=0;
                SMI_Barrier();
                i=0;
                while(i+k*s_proc_val < param1)
                {	if(flocal[i+k*s_proc_val])
                        {	if((i+k*s_proc_val < my_bottom) 
                                || (i+k*s_proc_val > my_top))
                                {	remote=(i+k*s_proc_val)/proc_val;
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
                                        fcache[remote][cache_area*block_id].position++;
                                        offset
                                        =fcache[remote][cache_area*block_id].position;
                                        fcache[remote][cache_area*block_id+offset]                                              .position=i+k*s_proc_val;
                                        fcache[remote][cache_area*block_id+offset]
                                        .value=flocal[i+k*s_proc_val];
                                }
                                else
                                   	fshared[i+k*s_proc_val]+=flocal[i+k*s_proc_val];
                        }
                        i++;
                        if(!(i%s_proc_val)) i+=jump;
                }
                SMI_Barrier();
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=fcache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	fshared[fcache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=fcache[_smi_my_proc_rank][i*cache_area+j].value;
                }
           }
           for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(fcache[i]));
        }
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
        {  for(k=0; k < scale; k++)
           {  	for(i=0; i < cache_entries; i+=cache_area)
                   	dcache[_smi_my_proc_rank][i].position=0;
                SMI_Barrier();
                i=0;
                while(i+k*s_proc_val < param1)
                {	if(dlocal[i+k*s_proc_val])
                        {	if((i+k*s_proc_val < my_bottom) 
                                || (i+k*s_proc_val > my_top))
                                {	remote=(i+k*s_proc_val)/proc_val;
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
                                        dcache[remote][cache_area*block_id].position++;
                                        offset
                                        =dcache[remote][cache_area*block_id].position;
                                        dcache[remote][cache_area*block_id+offset]                                              .position=i+k*s_proc_val;
                                        dcache[remote][cache_area*block_id+offset]
                                        .value=dlocal[i+k*s_proc_val];
                                }
                                else
                                   	dshared[i+k*s_proc_val]+=dlocal[i+k*s_proc_val];
                        }
                        i++;
                        if(!(i%s_proc_val)) i+=jump;
                }
                SMI_Barrier();
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=dcache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	dshared[dcache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=dcache[_smi_my_proc_rank][i*cache_area+j].value;
                }
           }
           for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(dcache[i]));
        }

        free(icache);
        free(lcache);
        free(fcache);
        free(dcache);
        return SMI_SUCCESS;
}







/********************************************************************************/
/*** This is for the sparse case. However, it relies on the allocation of     ***/
/*** further shared memory in internal segments that grow eith problem size.  ***/
/********************************************************************************/
smi_error_t _smi_combine_add_sparse_large(region_t* shared, region_t* local, int param1,
				 int param2, int comb_mode)
{	int i, j; 
        int remote;		/* Rank of corresponding process when transfering */
        int block_id;		/* Block in remote cache to put data from local */
        int no_values; 		/* No of valid entries inside caching area */
        int proc_val		/* No of local processed values */
        =param1/_smi_nbr_procs;
        int cache_area		/* Size of one cache area */
        =proc_val+1;
	int cache_entries	/* Individual cache size */
        =(_smi_nbr_procs-1)*cache_area;
	int my_bottom		/* Lower limit of my cache window */
        =_smi_my_proc_rank*param1/_smi_nbr_procs;
	int my_top		/* Upper limit of my cache window */
        =(_smi_my_proc_rank+1)*param1/_smi_nbr_procs;        
        int offset;		/* Relative R/W position in current cache block */
	smi_error_t error;

   	/* Declare struct for caching different vector data types */
	struct i_cache
	{	int position;
		int value;
	};
	struct l_cache
	{	int position;
		long int value;
	};
	struct f_cache
	{	int position;
		float value;
	};
	struct d_cache
	{	int position;
		double value;
        };

   	/* Some casts and cache vars */        
        int* ilocal		= (int*)local->addresses[0];
        int* ishared		= (int*)shared->addresses[0];
        struct i_cache** icache;
        long int* llocal	= (long int*)local->addresses[0];
        long int* lshared	= (long int*)shared->addresses[0];
        struct l_cache** lcache;
        float* flocal		= (float*)local->addresses[0];
        float* fshared		= (float*)shared->addresses[0];
        struct f_cache** fcache;
        double* dlocal		= (double*)local->addresses[0];
        double* dshared		= (double*)shared->addresses[0];
        struct d_cache** dcache;
        
   	/* Arrays for pointers to different cache areas */
        ALLOCATE( icache, struct i_cache**, _smi_nbr_procs * sizeof(struct i_cache*) );
        ALLOCATE( lcache, struct l_cache**, _smi_nbr_procs * sizeof(struct l_cache*) );
        ALLOCATE( fcache, struct f_cache**, _smi_nbr_procs * sizeof(struct f_cache*) );
        ALLOCATE( dcache, struct d_cache**, _smi_nbr_procs * sizeof(struct d_cache*) );
        
	/* Initializing shared region and cache area type depending */
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) ishared[i]=0;
           	for(i=0; i < _smi_nbr_procs; i++)
                   	error=SMI_Cmalloc(sizeof(struct i_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(icache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_large proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
           	/* Mark all caching fields as empty */
                for(i=0; i < cache_entries; i+=cache_area)                                                     	icache[_smi_my_proc_rank][i].position=0;
        }
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) lshared[i]=0;
           	for(i=0; i < _smi_nbr_procs; i++)
                   	error=SMI_Cmalloc(sizeof(struct l_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(lcache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_large proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
           	for(i=0; i < cache_entries; i+=cache_area)
                   	lcache[_smi_my_proc_rank][i].position=0;
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) fshared[i]=(float)0.0;
           	for(i=0; i < _smi_nbr_procs; i++)
                   	error=SMI_Cmalloc(sizeof(struct f_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(fcache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_large proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
           	for(i=0; i < cache_entries; i+=cache_area)
                   	fcache[_smi_my_proc_rank][i].position=0;
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
        {	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) dshared[i]=0.0;
                for(i=0; i < _smi_nbr_procs; i++)
                  	error=SMI_Cmalloc(sizeof(struct d_cache)*cache_entries,                                                     	i|INTERNAL, (char**)&(dcache[i]));
                if(error) 
				{
					fprintf(stderr,"_smi_combine_add_sparse_large proc=%i error=%i\n", _smi_my_proc_rank, error);
					return(error);
				}
                for(i=0; i < cache_entries; i+=cache_area)
                   	dcache[_smi_my_proc_rank][i].position=0;
	}
	
	SMI_Barrier();

/* This routine is especially usefull when adding sparse vectors. Every process */
/* scans his own vector to find valid elements. If these are in his own shared 	*/
/* mem area they are added immediately, otherwise they are transfered via SCI to*/
/* corresponding remote process. In this case just the element together with 	*/
/* his local position is copied. A barrier follows to close up local operation. */ 
/* During next cycle the transfered data is elementwise locally added and 	*/
/* finally parts are added to shared area.					*/

	if((comb_mode & SMI_DTYPE_FIXPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
        {	/* Scanning all entries of local replication */
                for(i=0; i < param1; i++)
                {	if(ilocal[i])
                        {	if((i < my_bottom) || (i > my_top))
                                {	/* ID of remote process */
                                        remote=i/proc_val;
                                        /* Block id in remote cache is determined */
                                        /* from distance between blocks */
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
					icache[remote][cache_area*block_id].position++;
                                        offset
                                        =icache[remote][cache_area*block_id].position;
                                        icache[remote][cache_area*block_id+offset]                                              .position=i;
                                        icache[remote][cache_area*block_id+offset]
                                        .value=ilocal[i];
                                }
                                else
                                   	ishared[i]+=ilocal[i];
                        }
                }
                SMI_Barrier();
                /* Adding the cached elements to shared vector */
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=icache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	ishared[icache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=icache[_smi_my_proc_rank][i*cache_area+j].value;
                }
                for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(icache[i]));
        }
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
        {	for(i=0; i < param1; i++)
                {	if(llocal[i])
                        {	if((i < my_bottom) || (i > my_top))
                                {	remote=i/proc_val;
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
                                        lcache[remote][cache_area*block_id].position++;
                                        offset
                                        =lcache[remote][cache_area*block_id].position;
                                        lcache[remote][cache_area*block_id+offset]                                              .position=i;
                                        lcache[remote][cache_area*block_id+offset]
                                        .value=llocal[i];
                                }
                                else
                                   	lshared[i]+=llocal[i];
                        }
                }
                SMI_Barrier();
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=lcache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	lshared[lcache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=lcache[_smi_my_proc_rank][i*cache_area+j].value;
                }
                for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(lcache[i]));
        }
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
        {	for(i=0; i < param1; i++)
                {	if(flocal[i])
                        {	if((i < my_bottom) || (i > my_top))
                                {	remote=i/proc_val;
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
                                        fcache[remote][cache_area*block_id].position++;
                                        offset
                                        =fcache[remote][cache_area*block_id].position;
                                        fcache[remote][cache_area*block_id+offset]                                              .position=i;
                                        fcache[remote][cache_area*block_id+offset]
                                        .value=flocal[i];
                                }
                                else
                                   	fshared[i]+=flocal[i];
                        }
                }
                SMI_Barrier();
                /* Adding the cached elements to shared vector */
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=fcache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	fshared[fcache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=fcache[_smi_my_proc_rank][i*cache_area+j].value;
                }
                for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(fcache[i]));
        }
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
        {	for(i=0; i < param1; i++)
                {	if(dlocal[i])
                        {	if((i < my_bottom) || (i > my_top))
                                {	remote=i/proc_val;
                                        block_id
                                        =(_smi_nbr_procs+_smi_my_proc_rank-remote-1)
                                        %_smi_nbr_procs;
                                        dcache[remote][cache_area*block_id].position++;
                                        offset
                                        =dcache[remote][cache_area*block_id].position;
                                        dcache[remote][cache_area*block_id+offset]                                              .position=i;
                                        dcache[remote][cache_area*block_id+offset]
                                        .value=dlocal[i];
                                }
                                else
                                   	dshared[i]+=dlocal[i];
                        }
                }
                SMI_Barrier();
                /* Adding the cached elements to shared vector */
                for(i=0; i < _smi_nbr_procs-1; i++)
                {	no_values=dcache[_smi_my_proc_rank][i*cache_area].position;
                        for(j=1; j <= no_values; j++)
                           	dshared[dcache[_smi_my_proc_rank][i*cache_area+j].position]
                                +=dcache[_smi_my_proc_rank][i*cache_area+j].value;
                }
                for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(dcache[i]));
        }

        free(icache);
        free(lcache);
        free(fcache);
        free(dcache);
        return SMI_SUCCESS;
}





/********************************************************************************/
/*** This is a version for the non-sparse case. However, it _smi_allocates further ***/
/*** shared memory in internal segments that gows with the vector sizes.      ***/
/********************************************************************************/
smi_error_t _smi_combine_add_dense_large(region_t* shared, region_t* local, int param1,
			  int param2, int comb_mode)
{	int i, j; 
	int length=param1/_smi_nbr_procs;	/* Elements per process */ 
	int area=length*_smi_my_proc_rank; 	/* Individual start rank */
	int size;			/* Bytes for current type */

	/* Some casts and cache vars */        
	int* ilocal  	= (int*)local->addresses[0];
	int* ishared 	= (int*)shared->addresses[0];
	int** icache;
        long int* llocal  = (long int*)local->addresses[0];
	long int* lshared = (long int*)shared->addresses[0];
	long int** lcache;
	float* flocal  	= (float*)local->addresses[0];
	float* fshared 	= (float*)shared->addresses[0];
	float** fcache;
	double* dlocal  = (double*)local->addresses[0];
	double* dshared = (double*)shared->addresses[0];
	double** dcache;
	smi_error_t error;

	/* Initializing shared region */
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) ishared[i]=0;
		size=sizeof(int);
	}
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) lshared[i]=0;
		size=sizeof(long int);
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) fshared[i]=(float)0.0;
		size=sizeof(float);
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	if(!(_smi_my_proc_rank)) for(i=0; i < param1; i++) dshared[i]=0.0;
		size=sizeof(double);
	}  

	/* Arrays for pointers to different cache areas */
	ALLOCATE( icache, int**, _smi_nbr_procs * sizeof(int*) );
	ALLOCATE( lcache, long int**, _smi_nbr_procs * sizeof(long int*) );
	ALLOCATE( fcache, float**, _smi_nbr_procs * sizeof(float*) );
	ALLOCATE( dcache, double**, _smi_nbr_procs *sizeof(double*) );
        
	/* Allocate individual caching mem with global addresses */
	for(i=0; i < _smi_nbr_procs; i++)
	{	error=SMI_Cmalloc(size*length, i|INTERNAL, (char**)&(icache[i]));
		if(error) 
		{	fprintf(stderr,">>>>%i %i\n",size,length);
			fprintf(stderr,"_smi_combine_add_dense_large proc=%i error=%i", _smi_my_proc_rank, error);
			return(error);
		}
		lcache[i] = (long int*)(icache[i]);
		fcache[i] = (float*)(icache[i]);
		dcache[i] = (double*)(icache[i]);
	}
  
	SMI_Barrier();

/* Every process adds the corresponding parts from other p's to his local copy	*/
/* from shared memory. To allow efficient SCI transactions, each process has a 	*/
/* caching area, where the correspondent process writes the necessary data	*/
/* before add operation.							*/

	if((comb_mode & SMI_DTYPE_FIXPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{	/* Serve all neighbours */
		for(i=1; i < _smi_nbr_procs; i++)
		{	/* Transfer all local elements to one neighbour process */
			for(j=0; j < length; j++)
				/* Transfer my raw data */
				icache[(i+_smi_my_proc_rank)%_smi_nbr_procs][j]
				=ilocal[length*((i+_smi_my_proc_rank)%_smi_nbr_procs)+j];
			/* Wait for finishing SCI actions */
			SMI_Barrier();
			/* Add cached values */
			for(j=0; j < length; j++) 
				ilocal[area+j]+=icache[_smi_my_proc_rank][j];
                        SMI_Barrier();
		}
		/* Add own values to shared vector */
		for(j=0; j < length; j++) ishared[area+j]+=ilocal[area+j]; 
	}
	if((comb_mode & SMI_DTYPE_FIXPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
	{  	for(i=1; i < _smi_nbr_procs; i++)
		{	for(j=0; j < length; j++)
				lcache[(i+_smi_my_proc_rank)%_smi_nbr_procs][j]
				=llocal[length*((i+_smi_my_proc_rank)%_smi_nbr_procs)+j];
			SMI_Barrier();
			for(j=0; j < length; j++)
				llocal[area+j]+=lcache[_smi_my_proc_rank][j];
                        SMI_Barrier();
		}
		for(j=0; j < length; j++) lshared[area+j]+=llocal[area+j];	
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (~comb_mode & SMI_DTYPE_HIGHPRECISION))
	{  	for(i=1; i < _smi_nbr_procs; i++)
		{	for(j=0; j < length; j++)
				fcache[(i+_smi_my_proc_rank)%_smi_nbr_procs][j]
				=flocal[length*((i+_smi_my_proc_rank)%_smi_nbr_procs)+j];
			SMI_Barrier();
			for(j=0; j < length; j++) 
				flocal[area+j]+=fcache[_smi_my_proc_rank][j];
                        SMI_Barrier();
		}
		for(j=0; j < length; j++) fshared[area+j]+=flocal[area+j];	
	}
	if((comb_mode & SMI_DTYPE_FLOATINGPOINT) && (comb_mode & SMI_DTYPE_HIGHPRECISION))
	{  	for(i=1; i < _smi_nbr_procs; i++)
		{	for(j=0; j < length; j++)
				dcache[(i+_smi_my_proc_rank)%_smi_nbr_procs][j]
				=dlocal[length*((i+_smi_my_proc_rank)%_smi_nbr_procs)+j];
			SMI_Barrier();
                        for(j=0; j < length; j++)
                           	dlocal[area+j]+=dcache[_smi_my_proc_rank][j];
                        SMI_Barrier();
		}
		for(j=0; j < length; j++) dshared[area+j]+=dlocal[area+j];
	}
	
	for(i=0; i < _smi_nbr_procs; i++) SMI_Cfree((char*)(icache[i]));
	free(icache);
	free(fcache);
	free(dcache);
	free(lcache);
	return SMI_SUCCESS;
}








/********************************************************************************/
/*** This is a slow version. But it works in all circumstances because it     ***/
/*** does not rely on additional shared memory space                          ***/
/********************************************************************************/
smi_error_t _smi_combine_add_dense_small(region_t* shared, region_t* local, int param1,
				int param2, int comb_mode)
 {
   int i, j;
   void* local_array  = local->addresses[0];
   void* shared_array = shared->addresses[0];


   /* Process 0 sets all elements in the shared array to zero */
   
   SMI_Barrier();
   
   if (_smi_my_proc_rank==0)
    {
      if (comb_mode && SMI_DTYPE_FLOATINGPOINT)
       {
	 if (comb_mode && SMI_DTYPE_HIGHPRECISION)
	    for(j=0;j<param1;j++)
	       ((double*)shared_array)[j] = 0.0;
	 else
	    for(j=0;j<param1;j++)
	       ((float*)shared_array)[j] = (float)0.0;
       }
      if (comb_mode && SMI_DTYPE_FIXPOINT)
       {
	 if (comb_mode && SMI_DTYPE_HIGHPRECISION)
	    for(j=0;j<param1;j++)
	       ((long int*)shared_array)[j] = 0;
	 else
	    for(j=0;j<param1;j++)
	       ((int*)shared_array)[j] = 0;
       }
    }
   


   /* one process after another adds it's local array */
   /* element-by-element to the shared array          */
   
   for(i=0;i<_smi_nbr_procs;i++)
    {
      SMI_Barrier();
      if (i==_smi_my_proc_rank)
       {
	 if (comb_mode && SMI_DTYPE_FLOATINGPOINT)
	  {
	    if (comb_mode && SMI_DTYPE_HIGHPRECISION)
	       for(j=0;j<param1;j++)
		  ((double*)shared_array)[j] += ((double*)local_array)[j];
	    else
	       for(j=0;j<param1;j++)
		  ((float*)shared_array)[j] += ((float*)local_array)[j];
	  }
	 if (comb_mode && SMI_DTYPE_FIXPOINT)
	  {
	    if (comb_mode && SMI_DTYPE_HIGHPRECISION)
	       for(j=0;j<param1;j++)
		  ((long int*)shared_array)[j] += ((long int*)local_array)[j];
	    else
	       for(j=0;j<param1;j++)
		  ((int*)shared_array)[j] += ((int*)shared_array)[j];
	  }
       }
    }

   
   SMI_Barrier();
   
   return(SMI_SUCCESS);
 }



 
  
/********************************************************************************/
/***                                                                          ***/
/*** Add element-wise local arrays of all participating processes that are    ***/
/*** contained in each process'  region "local"  so that the result is        ***/
/*** afterwards contained in the globally shared region "shared".             ***/
/***                                                                          ***/
/*** int comb_mode  'ored' flags that describe what to do in mode detail:     ***/
/***                FLOATINGPOINT    : the values are floating point quantities        ***/
/***                FIXPOINT : all values are integer quantities               ***/
/***                HIGHPRECISION    : 8-byte values (if not set: 4-byte values        ***/
/***                SPARSE  : each process array contains just a small        ***/
/***                          fraction of non-zero values                     ***/
/***                                                                          ***/
/***                in all these above cases, param1 states the total number  ***/
/***                elements in the arrays, starting right at the beginning   ***/
/***                of the regions                                            ***/
/***                                                                          ***/
/***                BAND    : in each                                         ***/
/********************************************************************************/
smi_error_t _smi_combine_add(region_t* shared, region_t* local, int param1,
		    int param2, int comb_mode)
 {
   if(param1<=0) return(SMI_SUCCESS);

   if (comb_mode & SMI_SHR_SPARSE)
    {
      if (param1<16384)
	 return(_smi_combine_add_sparse_large(shared, local, param1, param2, comb_mode));
      else
	 return(_smi_combine_add_sparse_small(shared, local, param1, param2, comb_mode));
    }
   else
    {
      if (param1<16384)
	 return(_smi_combine_add_dense_large(shared, local, param1, param2, comb_mode));
      else
	 return(_smi_combine_add_dense_small(shared, local, param1, param2, comb_mode));
    }
 }




 


