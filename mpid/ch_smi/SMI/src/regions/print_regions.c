/* $Id$ */

#include "print_regions.h"


  
void _smi_print_segment(shseg_t* seg)
{
   fprintf(stderr,"    Size = %i\n",seg->size);
   fprintf(stderr,"    Address = %p\n",seg->address);
   fprintf(stderr,"    Machine = %i\n",seg->machine);
   fprintf(stderr,"    Owning Process = %i\n",seg->owner);
   fprintf(stderr,"    Device = ");
   if(seg->device & DEV_GLOBAL)
      fprintf(stderr," SCI\n");
   if(seg->device == DEV_SMP)
      fprintf(stderr," SMP shared segment\n");
   if(seg->device == DEV_LOCAL)
      fprintf(stderr," local memory\n");
#ifndef NO_SISCI
#ifdef _WIN64
	fprintf(stderr,"    ID/FD = %i/%I64l\n\n", (int) seg->id, seg->fd);
#else
   fprintf(stderr,"    ID/FD = %i/%lu\n\n", (int) seg->id, (unsigned long) seg->fd);
#endif
#else /* NO_SISCI */
   fprintf(stderr,"    ID = %i\n\n", (int) seg->id);
#endif /* NO_SISCI */
}
  



/****************************************************************************/
/*** For debug purposes only, this functions prints all information about ***/
/*** regions to stderr.                                                   ***/
/****************************************************************************/
void _smi_print_regions(void)
{
    int i, j;
    
    SMI_LOCK(&_smi_mis_lock);
    
    fprintf(stderr,"\nTotal Number of Regions = %i\n\n",_smi_mis.no_regions);
    
    for(i=0;i<_smi_mis.no_regions;i++)
    {
	fprintf(stderr,"  Region Id = %i\n",_smi_mis.region[i]->id);
	fprintf(stderr,"  Counterpart Region Id = ");
	if(_smi_mis.region[i]->counterpart_id == -1)
	    fprintf(stderr,"none\n");
	else
	    fprintf(stderr,"%i\n",_smi_mis.region[i]->counterpart_id);
	fprintf(stderr,"  Total size = %i\n", _smi_mis.region[i]->size);
	fprintf(stderr,"  Number of Segments = %i\n", _smi_mis.region[i]->no_segments);
	fprintf(stderr,"  Start Address= %p\n", _smi_mis.region[i]->addresses[0]);
	fprintf(stderr,"  replication= %x\n\n", _smi_mis.region[i]->replication);
	
	
	
	for(j=0;j<_smi_mis.region[i]->no_segments;j++)
	{
	    fprintf(stderr,"    Segment %i\n",j);
	    _smi_print_segment(_smi_mis.region[i]->seg[j]);
	}
	
	
	fprintf(stderr,"\n\n");
    }
    
    fprintf(stderr,"\n\n");
    
    SMI_UNLOCK(&_smi_mis_lock);
}



  
