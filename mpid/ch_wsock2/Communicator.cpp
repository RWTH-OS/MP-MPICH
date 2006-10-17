#include <malloc.h>
#include "communicator.h"

/*! \file Communicator.cpp
 \brief This file contains the implementation of the Communicator
*/

/*!
\class CCommunicator Communicator.h
\brief The CCommunicator class is intended to serve as base for the communication subsystem.


  This class allows you to implement your own communication subsystem.
  It assumes a fully connected network with point-to-point connections 
  represented by instances of CCommPoint.
  It allows you to query the number of processes and your own number.
  Also a simple barrier and a broadcast algorithm are implemented.
  CCommunicator normally uses a second thread to handle incoming 
  messages. These are dipatched by the DispatchMessage() member.
  Each message sent within the system must contain a MessageHeader 
  structure. This will be extracted by the DispatchMessage() member of the
  decendant and delivered to CCommunicator::DispatchMessage().

*/


extern DWORD TlsIndex;

/*!
 This variable points to the currently used communicator.
 This will always be a class derived from CCommunicator.
 Currently we use CTCPCommunicator.
*/
CCommunicator *comm = 0;


