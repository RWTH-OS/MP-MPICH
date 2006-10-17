#ifndef __NTREXEC_H__
#define __NTREXEC_H__

#include "portmapper.h"
#include "userenv.h"

#define ProcessCreate 1
#define ProcessTerminate 2
#define EndConnection 3
#define ProcessCreated 4
#define CreationFailed 5
#define InfoText 6


struct NtrexecMessage {
	int Command;
	char  Commandline[MAX_PATH],
		  CurrentDir[MAX_PATH],
		  StdinName[MAX_PATH],
		  StdoutName[MAX_PATH],
		  StderrName[MAX_PATH],
		  Domain[MAX_PATH],
		  UserName[MAX_USER_NAME_LEN],
		  Password[MAX_PATH];
};


HANDLE MyLoadUserProfile(HANDLE User,NtrexecMessage *msg,UserInfo& UI);

			  	

#endif
