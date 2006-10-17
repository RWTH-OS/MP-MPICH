/* $Id: err.h,v 1.1 2004/03/19 22:14:15 joachim Exp $ */

#ifndef __ERR_H
#define __ERR_H

#ifdef WIN32
#pragma warning (disable: 4290) // weil ich mich wichtig machen moechte 
// Avoid messageboxes when critical errors occur
// SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX),
#endif


#include <iostream>



// define DEBUG macro; macro is empty if SHOW_DEBUG==false 
#ifdef SHOW_DEBUG
   #include <fstream>
   #define DEBUG(t) {file<<t;}
#else
   #define DEBUG(t) {}
#endif
//------------------------------------------------------------------------
///////////////
// err class //
///////////////
//------------------------------------------------------------------------
class err 
{
private:
   int	code;
   int	location;
   char*	text;
public:
   // constructor
   inline err(char* t,int l=0,int c=0):text(t),location(l),code(c) {}
   // returns error code
   inline int error() {return(code);}
   // overloaded output stream operator for error output; output only if debug==true
   friend std::ostream& operator<<(std::ostream& out,err e);
};
//------------------------------------------------------------------------
///////////////
// prototype //
///////////////  
//------------------------------------------------------------------------
#ifdef SHOW_DEBUG
void _smi_openDebugFile(ofstream *file,int num) throw(err);
#endif

#endif

