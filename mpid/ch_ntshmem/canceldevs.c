#include "mpid.h"
#include "mpiddev.h"


int expect_cancel_ack = 0;

#if 0
/* This routine is called from adi2cancel
   it is needed for multi device configurations*/
void MPID_SendCancelPacket( request, err_code )
MPI_Request *request;
int         *err_code;
{

	MPIR_SHANDLE *shandle = (MPIR_SHANDLE *)*request; 
	MPID_Device *dev = MPID_devset->dev[shandle->partner];
	int (*fcn) ANSI_ARGS((MPIR_SHANDLE *));
    
    /* Choose the function on a fcfs scheme */
    if (dev->short_msg->cancel_send)
	fcn = dev->short_msg->cancel_send;
    else if (dev->long_msg->cancel_send) 
	fcn = dev->long_msg->cancel_send;
    else
	fcn = dev->vlong_msg->cancel_send;
    DEBUG_TEST_FCN(fcn,"dev->proto->cancel_send");
    *err_code = (*(fcn))(shandle);
}
#endif
/* This routine will block while a process is still expecting an ack from
   a cancel request.  Called by MPID_CH_End */
void MPID_FinishCancelPackets( dev )
MPID_Device *dev;

{  /* begin MPID_FinishCancelPackets */

    
    while (expect_cancel_ack > 0) {
	MPID_DeviceCheck( MPID_BLOCKING ); } 


}  /* end MPID_FinishCancelPackets */

