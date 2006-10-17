
/*
 |  These are the wrapper functions between ADI2 and the ch_device which set 
 |  the active_dev pointer in a multidevice environment: 
 |  (the respective header are defined in dev.h)
*/

#include "dev.h"
#include "mpid.h"
#include "mpiddev.h"

/* Wrapper function for the device struct (MPID_Device): */

int MPID_Device_call_long_len ( int x, struct _MPID_Device *this )
{
  MPID_devset->active_dev = this;
  return this->long_len( x );
}

int MPID_Device_call_vlong_len( int x,  struct _MPID_Device *this )
{
  MPID_devset->active_dev = this;
  return this->vlong_len( x );
}

void *MPID_Device_call_alloc_mem(size_t x, MPID_Info* y,  struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return this->alloc_mem( x, y );
}

int MPID_Device_call_free_mem(void* x,  struct _MPID_Device *this)
{ 
  MPID_devset->active_dev = this;
  return this->free_mem( x );
}

int MPID_Device_call_check_device(MPID_Device* this, MPID_BLOCKING_TYPE y)
{
  MPID_devset->active_dev = this;
  return this->check_device( this, y );
}

int MPID_Device_call_terminate(MPID_Device* this)
{
  MPID_devset->active_dev = this;
  return this->terminate( this );
}

int MPID_Device_call_abort(struct MPIR_COMMUNICATOR * x, int y, char * z,  struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return this->abort( x, y, z );
}

int MPID_Device_call_cancel(MPIR_SHANDLE * x,  struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return this->cancel( x );
}

void MPID_Device_call_get_version(char * x,  struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  this->get_version( x );
}

int MPID_Device_call_persistent_init(union MPIR_HANDLE * x,  struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return this->persistent_init( x );
}

int MPID_Device_call_persistent_free(union MPIR_HANDLE * x,  struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return this->persistent_free( x );
}


/* Wrapper function for the protocol struct (MPID_Protocol): */

int MPID_Protocol_call_send(int(*func)(void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE *),
			    void *a, int b, int c, int d, int e, int f, MPID_Msgrep_t g, struct MPIR_DATATYPE * h,
			    struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return func(a, b, c, d, e, f, g, h);
}

int MPID_Protocol_call_recv(int(*func)(MPIR_RHANDLE *, int, void *),
			    MPIR_RHANDLE *a, int b, void *c,
			    struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return func(a, b, c);
}

int MPID_Protocol_call_isend(int(*func)(void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE *),
			     void *a, int b, int c, int d, int e, int f, MPID_Msgrep_t g, MPIR_SHANDLE *h, struct MPIR_DATATYPE *i,
			     struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return func(a, b, c, d, e, f, g, h, i);
}

int MPID_Protocol_call_irecv(int(*func)(MPIR_RHANDLE *, int, void *),
			     MPIR_RHANDLE *a, int b, void *c,
			     struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return func(a, b, c);
}

int MPID_Protocol_call_unex(int(*func)(MPIR_RHANDLE *, int, void *), MPIR_RHANDLE *a, int b, void * c, struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return func(a, b, c);
}

int MPID_Protocol_call_do_ack(int(*func)(void *, int), void* a, int b, struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  return func(a, b);
}

void MPID_Protocol_call_delete(void(*func)(MPID_Protocol *), MPID_Protocol *a, struct _MPID_Device *this)
{
  MPID_devset->active_dev = this;
  func(a);
}
