/* 
 *   $Id$    
 */
#include "mpiimpl.h"
#include "mpimem.h"
#include "mpipt2pt.h"

/* This file is currently NOT USEFUL - it requires changes and tests:
   ******************************
   DO NOT USE THIS CODE!!!
   ******************************

   Code for flattening datatypes in order to transfer them more efficiently 
   (without intermediate copies).

   For shared memory or SCI, a more efficient solution is implemented (direct_pack_ff).
   The technique used here may be of use for streaming communication like sockets.
*/

static void MPIR_Flatten(struct MPIR_DATATYPE *, MPIR_FLAT_BUF *,
		  MPI_Aint, int *)  ;




int MPIR_Type_flatten(struct MPIR_DATATYPE *dtypeptr)
{
    int curr_index=0;

    /* is it entirely contiguous? */
    if(dtypeptr->is_contig) {
	return 0;
    }

    if(dtypeptr->flattened) 
	return 1;

    if (dtypeptr->flatcount) {
	  MPIR_ALLOC(dtypeptr->flattened,(MPIR_FLAT_BUF *) MALLOC( dtypeptr->flatcount*sizeof(MPIR_FLAT_BUF) ),
	      MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, "MPIR_TYPE_FLATTEN" );
    } else return 0;
	
    curr_index = 0;

    MPIR_Flatten(dtypeptr, dtypeptr->flattened, 0, &curr_index);

    /* debug */
#if 0
    FPRINTF(stderr, "blens: ");
    for (i=0; i<flat->count; i++) 
	FPRINTF(stderr, "%d ", flat->blocklens[i]);
    FPRINTF(stderr, "\n\n");
    FPRINTF(stderr, "indices: ");
    for (i=0; i<flat->count; i++) 
	FPRINTF(stderr, "%ld ", flat->indices[i]);
    FPRINTF(stderr, "\n\n");
#endif

    return 1;
}

#define MAY_BE_NONCONTIG(d) (\
		      (!(d)->is_contig) && \
		      ( ((d)->dte_type == MPIR_HVECTOR) ||\
                        ((d)->dte_type == MPIR_HINDEXED) ||\
		        ((d)->dte_type == MPIR_STRUCT)\
		      )\
		    )

void MPIR_Flatten( struct MPIR_DATATYPE *act_type,MPIR_FLAT_BUF *flattened, 
		  MPI_Aint st_offset, int *curr_index)  
{
    int i, j, m, n, num, basic_num, prev_index;
    int top_count;

    switch (act_type->dte_type) {
    case MPIR_CONTIG:
	top_count = act_type->count;

	prev_index = *curr_index;
	if (MAY_BE_NONCONTIG(act_type->old_type))
	    MPIR_Flatten(act_type->old_type,flattened, st_offset, curr_index);

	if (prev_index == *curr_index) {
	    /* simplest case, made up of basic or contiguous types */
	    j = *curr_index;
	    flattened[j].buf = st_offset;
	    flattened[j].len = top_count * act_type->old_type->size;
	    (*curr_index)++;
	}
	else {
	    /* made up of noncontiguous derived types */
	    j = *curr_index;
	    num = *curr_index - prev_index;

	    /* The noncontiguous types have to be replicated count times */
	    for (m=1; m<top_count; m++) {
		for (i=0; i<num; i++) {
		    flattened[j].buf = flattened[j-num].buf + act_type->old_type->extent;
		    flattened[j].len = flattened[j-num].len;
		    j++;
		}
	    }
	    *curr_index = j;
	}
	break;

    case MPIR_HVECTOR: 
	top_count = act_type->count;

	prev_index = *curr_index;
	if (MAY_BE_NONCONTIG(act_type->old_type))
	    MPIR_Flatten(act_type->old_type,flattened, st_offset, curr_index);

	if (prev_index == *curr_index) {
	    /* simplest case, vector of basic or contiguous types */
	    j = *curr_index;
	    flattened[j].buf = st_offset;
	    flattened[j].len = act_type->blocklen * act_type->old_type->size;
	    for (i=j+1; i<j+top_count; i++) {
		flattened[i].buf=flattened[i-1].buf + act_type->stride;
		flattened[i].len=flattened[j].len;
	    }
	    *curr_index = i;
	}
	else {
	    /* vector of noncontiguous derived types */
	    j = *curr_index;
	    num = *curr_index - prev_index;

	    /* The noncontiguous types have to be replicated blocklen times
	       and then strided. Replicate the first one. */
	    for (m=1; m<act_type->blocklen; m++) {
		for (i=0; i<num; i++) {
		    flattened[j].buf = flattened[j-num].buf+act_type->old_type->extent;
		    flattened[j].len = flattened[j-num].len;
		    j++;
		}
	    }
	    *curr_index = j;

	    /* Now repeat with strides. */
	    num = *curr_index - prev_index;
	    for (i=1; i<top_count; i++) {
 		for (m=0; m<num; m++) {
		    flattened[j].buf = flattened[j-num].buf+act_type->stride;
		    flattened[j].len = flattened[j-num].len;
		   j++;
		}
	    }
	    *curr_index = j;
	}
	break;

    case MPIR_HINDEXED: 
	top_count = act_type->count;

	prev_index = *curr_index;
	if (MAY_BE_NONCONTIG(act_type->old_type))
	    MPIR_Flatten(act_type->old_type,flattened, st_offset+act_type->indices[0], curr_index); 

	if (prev_index == *curr_index) {
	    /* simplest case, indexed type made up of basic or contiguous types */
	    j = *curr_index;
	    for (i=j; i<j+top_count; i++) {
		flattened[i].buf=st_offset + act_type->indices[i-j];
		flattened[i].len=act_type->blocklens[i-j]*act_type->old_type->size;
	    }
	    *curr_index = i;
	}
	else {
	    /* indexed type made up of noncontiguous derived types */

	    j = *curr_index;
	    num = *curr_index - prev_index;
	    basic_num = num;

	    /* The noncontiguous types have to be replicated blocklens[i] times
	       and then strided. Replicate the first one. */
	    for (m=1; m<act_type->blocklens[0]; m++) {
		for (i=0; i<num; i++) {
		    flattened[j].buf = flattened[j-num].buf+act_type->old_type->extent;
		    flattened[j].len = flattened[j-num].len;
		    j++;
		}
	    }
	    *curr_index = j;

	    /* Now repeat with strides. */
	    for (i=1; i<top_count; i++) {
		num = *curr_index - prev_index;
		prev_index = *curr_index;
		for (m=0; m<basic_num; m++) {
		    flattened[j].buf = flattened[j-num].buf+act_type->indices[i];
		    flattened[j].len = flattened[j-num].len;
		    j++;
		}
		*curr_index = j;
		for (m=1; m<act_type->blocklens[i]; m++) {
		    flattened[j].buf = flattened[j-basic_num].buf+act_type->old_type->extent;
		    flattened[j].len = flattened[j-basic_num].len;
		    j++;
		}
		*curr_index = j;
	    }
	}
	break;

    case MPIR_STRUCT: 
	top_count = act_type->count;
	for (n=0; n<top_count; n++) {
	    prev_index = *curr_index;
            if (MAY_BE_NONCONTIG(act_type->old_types[n]))
		MPIR_Flatten(act_type->old_types[n],flattened, st_offset+act_type->indices[n], curr_index);

	    if (prev_index == *curr_index) {
		/* simplest case, current type is basic or contiguous types */
		j = *curr_index;
		flattened[j].buf = st_offset + act_type->indices[n]; 
		flattened[j].len = act_type->blocklens[n]*act_type->old_types[n]->size; 
		(*curr_index)++;
	    }
	    else {
		/* current type made up of noncontiguous derived types */
		j = *curr_index;
		num = *curr_index - prev_index;

		/* The current type has to be replicated blocklens[n] times */
		for (m=1; m<act_type->blocklens[n]; m++) {
		    for (i=0; i<num; i++) {
			flattened[j].buf = flattened[j-num].buf + act_type->old_types[n]->extent;
			flattened[j].len = flattened[j-num].len;
			j++;
		    }
		}
		*curr_index = j;
	    }
	}
 	break;
	
    default:
	MPIR_ERROR(MPIR_COMM_WORLD,
		   MPIR_ERRCLASS_TO_CODE(MPI_ERR_TYPE,MPIR_ERR_DEFAULT),
		   "MPIR_Flatten");
    }

}


/********************************************************/



int MPIR_Count_contiguous_blocks(struct MPIR_DATATYPE *datatype, int *curr_index)
{
    int count=0, i, n, num, basic_num, prev_index;
    int top_count;
    
    if (datatype->is_contig) {
      (*curr_index)++;
      return 1;
    }
    
    switch (datatype->dte_type) {
    case MPIR_CONTIG:
        top_count = datatype->count;
	prev_index = *curr_index;
	if (MAY_BE_NONCONTIG(datatype->old_type))
	    count = MPIR_Count_contiguous_blocks(datatype->old_type, curr_index);
	else count = 1;

	if (prev_index == *curr_index) 
	    /* simplest case, made up of basic or contiguous types */
	    (*curr_index)++;
	else {
	    /* made up of noncontiguous derived types */
	    num = *curr_index - prev_index;
	    count *= top_count;
	    *curr_index += (top_count - 1)*num;
	}
	break;

    case MPIR_VECTOR:
    case MPIR_HVECTOR:
        top_count = datatype->count;
        
	prev_index = *curr_index;
	if (MAY_BE_NONCONTIG(datatype->old_type))
	    count = MPIR_Count_contiguous_blocks(datatype->old_type, curr_index);
	else count = 1;

	if (prev_index == *curr_index) {
	    /* simplest case, vector of basic or contiguous types */
	    count = top_count;
	    *curr_index += count;
	}
	else {
	    /* vector of noncontiguous derived types */
	    num = *curr_index - prev_index;

	    /* The noncontiguous types have to be replicated blocklen times
	       and then strided. */
	    count *= datatype->blocklen * top_count;
	    
	    /* First one */
	    *curr_index += (datatype->blocklen - 1)*num;

	    /* Now repeat with strides. */
	    num = *curr_index - prev_index;
	    *curr_index += (top_count - 1)*num;
	}
	break;

    case MPIR_INDEXED: 
    case MPIR_HINDEXED:
        top_count = datatype->count;

	prev_index = *curr_index;
	if (MAY_BE_NONCONTIG(datatype->old_type))
	    count = MPIR_Count_contiguous_blocks(datatype->old_type, curr_index);
	else count = 1;

	if (prev_index == *curr_index) {
	    /* simplest case, indexed type made up of basic or contiguous types */
	    count = top_count;
	    *curr_index += count;
	}
	else {
	    /* indexed type made up of noncontiguous derived types */
	    basic_num = *curr_index - prev_index;

	    /* The noncontiguous types have to be replicated blocklens[i] times
	       and then strided. */
	    *curr_index += (datatype->blocklens[0]-1) * basic_num;
	    count *= datatype->blocklens[0];

	    /* Now repeat with strides. */
	    for (i=1; i<top_count; i++) {
		count += datatype->blocklens[i] * basic_num;
		*curr_index += datatype->blocklens[i] * basic_num;
	    }
	}
	break;

    case MPIR_STRUCT: 
        top_count = datatype->count;
	count = 0;
	for (n=0; n<top_count; n++) {
            
	    prev_index = *curr_index;
	    if (MAY_BE_NONCONTIG(datatype->old_types[n]))
	    count += MPIR_Count_contiguous_blocks(datatype->old_types[n], curr_index);

	    if (prev_index == *curr_index) {
		/* simplest case, current type is basic or contiguous types */
		count++;
		(*curr_index)++;
	    }
	    else {
		/* current type made up of noncontiguous derived types */
		/* The current type has to be replicated blocklens[n] times */

		num = *curr_index - prev_index;
		count += (datatype->blocklens[n]-1)*num;
		(*curr_index) += (datatype->blocklens[n]-1)*num;
	    }
	}
	break;
    default:
	return 1;
    }


    return count;
}

#if defined(WIN32) && !defined(_WIN64)
__declspec(naked)
int MPIR_copy_out_flat_asm(char *out,char *in,int num_elements,MPIR_FLAT_BUF *f,unsigned int fc,MPI_Aint ex) {
#define out esp+20
#define in out+4
#define num_elements out+8
#define f out+12
#define fc out+16
#define ex out+20
__asm {
	push	esi
	push	edi
	push	ebp
	push	ebx

	mov	ebp, [fc];
	test	ebp, ebp
	jbe	SHORT LEND

	mov	edx,[f];
	mov	edi,[out];
	mov	esi, DWORD PTR [in]
	
LTOP_OUTER:
	xor	eax, eax
	jmp	LSTART

align 4
LTOP_INNER:
	mov	esi, DWORD PTR [in]
LSTART:
	mov	ecx, DWORD PTR [edx+eax*8]
	add	esi, DWORD PTR [edx+eax*8+4]
	mov	ebx, ecx
	cmp	ebx, 32
	jbe	LDWORDS
	
	shr	ecx, 5
	jz	LDWORDS
	and	ebx,31

align 4
LLINE:
	movq	mm0,[esi]
	movq	mm1,[esi+8]
	movq	mm2,[esi+16]
	movq	mm3,[esi+24]
	add	esi,32
	
	movq	[edi],mm0
	movq	[edi+8],mm1
	movq	[edi+16],mm2
	movq	[edi+24],mm3
	add	edi,32
	dec	ecx
	jnz	LLINE

LQWORD_START:
	
	mov	ecx,ebx
	shr	ecx, 3
	jz	LDWORDS
	and	ebx,7

LDWORDS:
	mov	ecx, ebx
	shr	ecx,2
	rep movsd
	and	ebx,3

LBYTES:
	mov	ecx, ebx
	rep movsb

LEND_INNER:
	inc	eax
	cmp	eax, ebp
	jb	SHORT LTOP_INNER


	mov	esi, DWORD PTR [in]
	add	esi, DWORD PTR [ex]
	mov	eax, DWORD PTR [num_elements]
	dec	eax
	mov	DWORD PTR [in], esi
	mov	DWORD PTR [num_elements], eax
	jne	SHORT LTOP_OUTER
	emms
	jmp	LEND

LEND:
	pop	ebx
	pop	ebp
	pop	edi
	pop	esi
	mov	eax,1
	ret
    }
#undef out
#undef in
#undef num_elements 
#undef f
#undef fc 
#undef ex
}
#else
/* Unix implementation - XXX only a dummy so far */
int MPIR_copy_out_flat_asm(char *out,char *in,int num_elements,MPIR_FLAT_BUF *f,unsigned int fc,MPI_Aint ex) {
    return 0;
}
#endif

/* This packs num_elements elements of type dtypeptr from inbuf into outbuf
    Returns 1 if successful, 0 otherwise.
*/
int MPIR_Pack_flat_type(void *outbuf,void *inbuf,int num_elements,struct MPIR_DATATYPE* dtypeptr) {
    unsigned int fc=dtypeptr->flatcount;
    MPIR_FLAT_BUF *f=dtypeptr->flattened;
    char *in=(char*)inbuf,*out=(char*)outbuf;
    MPI_Aint ex=dtypeptr->extent;

    if(!f || !dtypeptr->flatcount || !num_elements) return 0;
#ifdef WIN32
    MPIR_copy_out_flat_asm(outbuf,inbuf,num_elements,dtypeptr->flattened,dtypeptr->flatcount,dtypeptr->extent);
#endif
    return 1;
}



/* 
    This packs a maximum of num_elements elements of type dtypeptr from inbuf into outbuf
    or maxlen bytes, whichever comes first. *inbuf will be modified to point to 
    the next byte to copy. *next_flatbuf will point to the next element in dtypeptr->flatbut.
   
    Returns the number of bytes copied or 0 on error.
    Can be used for rndv-style functions.
*/
int MPIR_Pack_flat_restricted(void *outbuf,void **inbuf,int *next_flatbuf,MPI_Aint maxlen,
			      struct MPIR_DATATYPE* dtypeptr) {

    unsigned int j;
    MPI_Aint left = maxlen;
    MPIR_FLAT_BUF *f=dtypeptr->flattened;
    char *in=(char*)*inbuf,*out=(char*)outbuf;

    if(!f) return 0;
    j = *next_flatbuf;
    while(left>0) {
	for(;j<dtypeptr->flatcount;++j) {
	    if((MPI_Aint)f[j].len <= left) {
		memcpy(out,in+f[j].buf,f[j].len);
		out += f[j].len;
		left -= f[j].len; 
	    } else {
		*next_flatbuf = j;
		*inbuf = in;
		return maxlen-left;
	    }
	}
	j = 0;
	in += dtypeptr->extent;
    }

    return maxlen-left;

}

#if defined(WIN32) && !defined(_WIN64)
__declspec(naked)
int MPIR_copy_in_flat_asm(char *out,char *in,int num_elements,MPIR_FLAT_BUF *f,unsigned int fc,MPI_Aint ex) {
#define out esp+20
#define in out+4
#define num_elements out+8
#define f out+12
#define fc out+16
#define ex out+20
__asm {
	push	esi
	push	edi
	push	ebp
	push	ebx

	mov	ebp, [fc];
	test	ebp, ebp
	jbe	SHORT LEND

	mov	edx,[f];
	mov	edi, DWORD PTR [out];
	mov	esi, DWORD PTR [in]
	
LTOP_OUTER:
	xor	eax, eax
	jmp	LSTART

align 4
LTOP_INNER:
	mov	edi, DWORD PTR [out]
LSTART:
	mov	ecx, DWORD PTR [edx+eax*8]
	add	edi, DWORD PTR [edx+eax*8+4]
	mov	ebx, ecx
	cmp	ebx, 32
	jbe	LDWORDS

	shr	ecx, 5
	jz	LDWORDS
	and	ebx,31

align 4
LLINE:
	movq	mm0,[esi]
	movq	mm1,[esi+8]
	movq	mm2,[esi+16]
	movq	mm3,[esi+24]
	add	esi,32
	
	movq	[edi],mm0
	movq	[edi+8],mm1
	movq	[edi+16],mm2
	movq	[edi+24],mm3
	add	edi,32
	dec	ecx
	jnz	LLINE

LDWORDS:
	mov	ecx, ebx
	shr	ecx,2
	rep movsd
	and	ebx,3
	
LBYTES:
	mov	ecx, ebx
	rep movsb
	inc	eax
	cmp	eax, ebp
	jb	SHORT LTOP_INNER


	mov	edi, DWORD PTR [out]
	add	edi, DWORD PTR [ex]
	mov	eax, DWORD PTR [num_elements]
	dec	eax
	mov	DWORD PTR [out], edi
	mov	DWORD PTR [num_elements], eax
	jne	SHORT LTOP_OUTER

LEND:
	emms
	pop	ebx
	pop	ebp
	pop	edi
	pop	esi
	mov	eax,1
	ret
    }
#undef out
#undef in
#undef num_elements 
#undef f
#undef fc 
#undef ex
}
#endif

int MPIR_Unpack_flat_type(void *outbuf,void *inbuf,int num_elements,struct MPIR_DATATYPE* dtypeptr) {
    int i;
    unsigned int j,fc=dtypeptr->flatcount;
    MPIR_FLAT_BUF *f=dtypeptr->flattened;
    char *in=(char*)inbuf,*out=(char*)outbuf;
    MPI_Aint ex=dtypeptr->extent;

    if(!f || !dtypeptr->flatcount || !num_elements) return 0;
    /*memcpy(outbuf,inbuf,num_elements*dtypeptr->size);*/
    /*MPIR_copy_in_flat_asm(outbuf,inbuf,num_elements,dtypeptr->flattened,dtypeptr->flatcount,dtypeptr->extent);*/
    
    for(i=0;i<num_elements;++i) {
	for(j=0;j<fc;++j) {
	    memcpy(out+f[j].buf,in,f[j].len);
	    in += f[j].len;
	}
	out += ex;
    }
    return 1;
}

int MPIR_Unpack_flat_restricted(void **outbuf,void *inbuf,int *next_flatbuf,MPI_Aint maxlen,
			        struct MPIR_DATATYPE* dtypeptr) {
    unsigned int j;
    MPI_Aint left = maxlen;
    MPIR_FLAT_BUF *f=dtypeptr->flattened;
    char *in=(char*)inbuf,*out=(char*)*outbuf;

    if(!f) return 0;
    j = *next_flatbuf;
    while(left>0) {
	for(;j<dtypeptr->flatcount;++j) {
	    if ((MPI_Aint)f[j].len < left) {
		memcpy(out+f[j].buf,in,f[j].len);
		in += f[j].len;
		left -= f[j].len; 
	    } else {
		*next_flatbuf = j;
		*outbuf = out;
		return maxlen-left;
	    }
	}
	j = 0;
	out += dtypeptr->extent;
    }
    return maxlen-left;
}

int MPIR_Copy_flat_type(void *outbuf,void *inbuf,int num_elements,struct MPIR_DATATYPE* dtypeptr) {
    int i;
    unsigned int j;
    MPIR_FLAT_BUF *f=dtypeptr->flattened;
    char *in=(char*)inbuf,*out=(char*)outbuf;

    if(!f) return 0;
    for(i=0;i<num_elements;++i) {
	for(j=0;j<dtypeptr->flatcount;++j) {
	    memcpy(out+f[j].buf,in+f[j].buf,f[j].len);
	}
	in += dtypeptr->extent;
	out += dtypeptr->extent;
    }
    return 1;
}

int MPIR_Replicate_flat_descriptor(MPIR_FLAT_BUF **out,MPI_Aint offset,int num_elements,struct MPIR_DATATYPE* dtypeptr) {
    int i;
    unsigned int j;
    MPIR_FLAT_BUF *f=dtypeptr->flattened,*pos;
    if(!f) return 0;


     MPIR_ALLOC(pos,(MPIR_FLAT_BUF *) MALLOC( num_elements*dtypeptr->flatcount*sizeof(MPIR_FLAT_BUF) ),
	      MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, "MPIR_Replicate_flat_descriptor" );

     *out = pos;

     for(i=0;i<num_elements;++i) {
	for(j=0;j<dtypeptr->flatcount;++j) {
	    pos[j].buf = f[j].buf+offset;
	    pos[j].len = f[j].len;
	}
	pos += dtypeptr->flatcount;
	offset += dtypeptr->extent;
    }
    return 1;
}


