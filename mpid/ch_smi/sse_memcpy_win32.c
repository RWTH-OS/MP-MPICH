/*	$Id$

	Fast remote SCI copy for systems with write-combining enabled.
*/

#ifdef WIN32

#include <mmintrin.h>
#include <xmmintrin.h>

/* Fast remote SCI copy for systems with write-combining enabled.
   This is the version using SSE instructions to copy 128 Byte blocks,
   and flushes after 32 Byte. */
void _mpid_smi_sse32_memcpy(void *dest, const void *src, size_t size)
{
	char*	a = (char*) src;
	char*	b = (char*) dest;
	size_t	j = 0;
	__m128	xmm[8];

	/* Align the destination to a 64 Byte boundary */
	for(; (j < size) && (((size_t) &b[j]) % 64 != 0); j++)
		((char*) b)[j] = ((char*) a)[j];
		
	// Loads two cache lines of data to a location closer to the processor.
	_mm_prefetch(a+j, _MM_HINT_NTA);
	_mm_prefetch(a+j+64, _MM_HINT_NTA);

	/* copy 128 byte per loop */
	for (; (j+128) < size; j+=128) 
	{
		// Loads two cache lines of data to a location closer to the processor.
		_mm_prefetch(a+j+128, _MM_HINT_NTA);
		_mm_prefetch(a+j+192, _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_ps((float*) &a[j]);
		xmm[1] = _mm_load_ps((float*) &a[j+16]);
		xmm[2] = _mm_load_ps((float*) &a[j+32]);
		xmm[3] = _mm_load_ps((float*) &a[j+48]);
		xmm[4] = _mm_load_ps((float*) &a[j+64]);
		xmm[5] = _mm_load_ps((float*) &a[j+80]);
		xmm[6] = _mm_load_ps((float*) &a[j+96]);
		xmm[7] = _mm_load_ps((float*) &a[j+112]);

		/* store 32 byte */
		_mm_stream_ps((float*) &b[j], xmm[0]);
		_mm_stream_ps((float*) &b[j+16], xmm[1]);

		/* flush the write-combine buffer */
		_mm_sfence(); 

		/* store 32 byte */
		_mm_stream_ps((float*) &b[j+32], xmm[2]);
		_mm_stream_ps((float*) &b[j+48], xmm[3]);
		
		/* flush the write-combine buffer */
		_mm_sfence(); 

		/* store 32 byte */
		_mm_stream_ps((float*) &b[j+64], xmm[4]);
		_mm_stream_ps((float*) &b[j+80], xmm[5]);

		/* flush the write-combine buffer */
		_mm_sfence(); 
	
		/* store 32 byte */
		_mm_stream_ps((float*) &b[j+96], xmm[6]);
		_mm_stream_ps((float*) &b[j+112], xmm[7]);

		/* flush the write-combine buffer */
		_mm_sfence();  
	}

	/* copy tail */
	for(; j<size; j++)
		((char*) b)[j] = ((char*) a)[j];
}

/* Fast remote SCI copy for systems with write-combining enabled.
   This is the version using SSE instructions to copy 128 Byte blocks,
   and flushes after 64 Byte. */
void _mpid_smi_sse64_memcpy(void *dest, const void *src, size_t size)
{
	char*	a = (char*) src;
	char*	b = (char*) dest;
	size_t	j = 0;
	__m128	xmm[8];

	/* Align the destination to a 64 Byte boundary */
	for(; (j < size) && (((size_t) &b[j]) % 64 != 0); j++)
		((char*) b)[j] = ((char*) a)[j];
	
	// Loads two cache lines of data to a location closer to the processor.
	_mm_prefetch(a+j, _MM_HINT_NTA);
	_mm_prefetch(a+j+64, _MM_HINT_NTA);

	/* copy 128 byte per loop */
	for (; (j+128) < size; j+=128) 
	{
		// Loads two cache lines of data to a location closer to the processor.
		_mm_prefetch(a+j+128, _MM_HINT_NTA);
		_mm_prefetch(a+j+192, _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_ps((float*) &a[j]);
		xmm[1] = _mm_load_ps((float*) &a[j+16]);
		xmm[2] = _mm_load_ps((float*) &a[j+32]);
		xmm[3] = _mm_load_ps((float*) &a[j+48]);
		xmm[4] = _mm_load_ps((float*) &a[j+64]);
		xmm[5] = _mm_load_ps((float*) &a[j+80]);
		xmm[6] = _mm_load_ps((float*) &a[j+96]);
		xmm[7] = _mm_load_ps((float*) &a[j+112]);

		/* store 64 byte */
		_mm_stream_ps((float*) &b[j], xmm[0]);
		_mm_stream_ps((float*) &b[j+16], xmm[1]);
		_mm_stream_ps((float*) &b[j+32], xmm[2]);
		_mm_stream_ps((float*) &b[j+48], xmm[3]);
		
		/* flush the write-combine buffer */
		_mm_sfence(); 

		/* store 64 byte */
		_mm_stream_ps((float*) &b[j+64], xmm[4]);
		_mm_stream_ps((float*) &b[j+80], xmm[5]);
		_mm_stream_ps((float*) &b[j+96], xmm[6]);
		_mm_stream_ps((float*) &b[j+112], xmm[7]);

		/* flush the write-combine buffer */
		_mm_sfence();  
	}

	/* copy tail */
	for(; j<size; j++)
		((char*) b)[j] = ((char*) a)[j];
}

#endif