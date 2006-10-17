#ifndef __TCP_COMMUNICATOR_H__
#define __TCP_COMMUNICATOR_H__

// A special Communicator implemented with TCP sockets. 

#include "insocket.h"

class inSocket;

#ifdef _MSC_VER
#pragma warning( 3: 4705 4706 )
#endif

#define MAX_NAME_LEN 255
#define BASEPORT 2112
#define DEV "ch_wsock2"


#define ACK_MASTER 3
#define COMM_CONNECT 4
#define Start 5
#define Quit 6
#define AckQuit 7
#define ErrorExit 8

/*!
\struct NodeDescription TCPCommunicator.h
\brief This struct is used by CTCPCommunicator at startup time.

Each process sends one of these structs to the startup manager and gets
back an array of these structs describing all nodes that contacted the manager
earlier.
*/
struct NodeDescription {
	//! The MPI id hof that process
	DWORD ID;

	//!The number of addresses in the array.
	DWORD IPCount;

	//! The port this process accepts connections on.
	unsigned short listeningPort;

	//!The IP addresses of the host.
	unsigned long address[1];
};

struct MessageHeader {
	//! The size of the data following this header
	DWORD BodySize;	
};

/*!
\struct TCPMessageBody TCPCommunicator.h
\brief This struct is used by CTCPCommunicator to exchange messages.

  Each message sent by CTCPCommunicator uses this struct as message body
  to exchange information.
*/
struct TCPMessageBody {

	//! The message type
	DWORD type;

	/*! If sent by the master this field contains
	    the number of processes. If sent by the client
	    it indicates if this is a connection to localhost */
	DWORD local;

	//! Used if the connection is local.
	DWORD ProcessId;
	unsigned long long Handles[2];
	
	//! An array of NodeDescrition structs
	NodeDescription Data;
};

/*!
\struct TCPCommMessage TCPCommunicator.h
\brief This struct is used by CTCPCommunicator to exchange messages.

  Each message sent by CTCPCommunicator uses this struct
  to exchange information.
*/
struct TCPCommMessage {
	//! The default constructor initializes the header
	TCPCommMessage() {header.BodySize=sizeof(body); body.local = FALSE;}
	//! The message header
	MessageHeader header;
	
	//! And the body
	TCPMessageBody body;
};

/*!
\class CTCPCommunicator TCPCommunicator.h
\brief the TCPCommunicator class is an abstract layer that encapsulates sockets.

  This class is used to exchange messages between processes using Berkeley TCP sockets.
  It overrides several member functions of CCommunicator.<BR>
  At startup this class parses the commandline. Using this information one node
  becomes the \e startup \e master. All others become clients.
  Each client first connects to the master and gets its id and a list of nodes
  with lower ids. It then waits for all clients with higher id to connect. 
  If these connections are established the node connects to all clients with lower
  ids.
*/
class CTCPCommunicator {
public:
	CTCPCommunicator();
	virtual ~CTCPCommunicator();
	int Create(int *argc,char*** argv);
	
//	int SendMessageTo(int dest,void *msg,unsigned size);
	int GetFDs(int **Dest);
		//! Return the global unique id of the caller. 
	inline DWORD GetMyId() {return myId;}

	inline DWORD GetSMPId() {return SMPId;}
	
	//! Return the number of communicating nodes. 
	inline DWORD GetNumProcs() {return numProcs;}
protected:
	int MasterConnect();
	int ClientConnect(unsigned long Master);
	void MasterSMP(inSocket *Client,TCPMessageBody *msg);
	void ClientSMP(inSocket *Client,TCPMessageBody *msg);
	void CloseSockets();
	
	//! An array that contains the communication endpoints to all other nodes.
	// for local processes (shmem communication) set to NULL 
	inSocket **hosts;

	//! The port the startup master listens on.
	short Masterport;

	//! Used at startup time
	DWORD numConnections;

	//! Used by the MainLoop() for the select() call.
	fd_set backup;

	//! A flag indicating that the system shuts down connections. Used by ShudDown().

	bool ShuttingDown;

	//! The global SVMlib process id. Used by GetMyId().
	DWORD myId;
	
	//! The total number of processes. Used by GetNumProcs().
	DWORD numProcs;

	//! Used by MagPie to determine clusters
	DWORD SMPId;

	bool LocalMaster;
	
	unsigned long masterAddress,localAddress;
};


#endif

