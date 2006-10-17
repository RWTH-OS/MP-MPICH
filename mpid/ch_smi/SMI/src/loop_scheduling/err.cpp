/* $Id: err.cpp,v 1.1 2004/03/19 22:14:15 joachim Exp $ */

#include "err.h"

#include <math.h>

#include "env/general_definitions.h"	
#include "env/smidebug.h"

// boolean debug
   
//------------------------------------------------------------------------
/////////////////////////////
// member functions of err //
/////////////////////////////
//------------------------------------------------------------------------
//
// The error output funtion
std::ostream& operator<<(std::ostream& out,err e) 
{
#ifdef _DEBUG
  if(D_DO_DEBUG==TRUE)
    out <<"ERROR: "<<e.text<<", at "<<e.location<<", code "<<e.code<<std::endl;
#endif
  return(out);
}
//------------------------------------------------------------------------
//////////////
// function //
//////////////
//------------------------------------------------------------------------
//
// Open's a debug file (SHOW_DEBUG==true); the filename for process 2 is "logfile02.txt", 
// for example
#ifdef SHOW_DEBUG
void _smi_openDebugFile(ofstream *file,int num) throw(err)
{
#ifdef WIN32

   char name[30]="c:\\exp_imp\\logfile  .txt";
   name[18]=(char)((int)(num/10)+48); 
   name[19]=(char)(num%10+48);

#else
   char name[14]="logfile  .txt";
   
   name[7]=(char)((int)(num/10)+48); 
   name[8]=(char)(num%10+48);
#endif

   
   file->open(name);
   if(!(*file))
      throw err("File open error",__LINE__,SMI_ERR_OTHER);
}
#endif


