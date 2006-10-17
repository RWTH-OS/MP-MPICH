#ifndef __COMMUNICATOR_H__
#define __COMMUNICATOR_H__

/*!
 \file Communicator.h 
 \brief Communicator.h contains definitions belonging to CCommunicator.

  In this file basically all data belonging to CCommunicator can be found.
*/


// The Communicator class is intended as abstract base class.
// It provides the logically fully connected communication
// infrastructure, generic message passing and a software broadcast
// primitive (sequential and fibotree based). 

// Each SVMlib node runs exactly one Communicator instance.

#include <wtypes.h>

#ifdef _DEBUG
#define DBG(m) printf( "%s\n",m);
#else
#define DBG(m) 
#endif

#ifndef _WIN32
#define __stdcall
#endif

#ifdef _MSC_VER
#pragma warning( 3 : 4705 4706 )
#pragma warning( disable : 4786 )
#endif


class CCommunicator {

public:
	/*!
	The default constructor. Does nothing interesting.
	*/
	CCommunicator() {
		myId=0,
		numProcs=0;
	}
	/*!
	The destructor. Just calls StopThread()
	*/
	virtual ~CCommunicator() {
		DBG("Entering ~CCommunicator()");
	}

	/*! According to the command line given in argv, initialize and
	set up the distributed communication infrastructure. 
	Intended to be overriden by derived classes. 
	Returns -1
	*/
	virtual int Create(int *argc,char*** argv) {return -1;}
    
	/*!
	Send a message of arbitrary size to the Communicator instance
	whose myId is set to dest.
	It is assumed that the underlying protocol used is "reliable".
	Returns the number of bytes actually sent with this call. 
	Intended to be overriden by derived classes. 
	Returns -1
	*/
	virtual int SendMessageTo(int dest,void *msg,unsigned size) {return -1;}
		
	/*!
	This method is called to shut down the communication
	subsystem gracefully. Intended to be overriden by derived classes.
	Causes an error message.
	The parameter \a error is used to tell the system that an error occured
	in order to do a more "rapid" shutdown.
	*/
	virtual void ShutDown(BOOL error=FALSE) {};
	

	//! Return the global unique id of the caller. 
	inline DWORD GetMyId() {return myId;}
	
	//! Return the number of communicating nodes. 
	inline DWORD GetNumProcs() {return numProcs;}
	
protected:





};

#endif