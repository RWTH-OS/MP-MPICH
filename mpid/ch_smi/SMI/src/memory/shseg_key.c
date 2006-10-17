/* $Id: shseg_key.c,v 1.1 2004/03/19 22:14:17 joachim Exp $ */

#include "env/general_definitions.h"
#include "shseg_key.h"

/*
  In the case that several SMI applications are executed on the
  same node, they all would use the same memory segments and therefore interfere
  with eachother. The reason for this is that they all use the same 'key' to
  allocate a memory segment.
  To avoid such problems, the following function modifies a given key
  into a key that ensures that each application uses different keys.
*/



/****************************************************************************/
/****************************************************************************/
int _smi_modify_key(int key)
{
#ifdef WIN32
    return (key + 100*GetCurrentProcessId());
#else
    return (key + 100*(int)getpid());
#endif
}



/****************************************************************************/
/****************************************************************************/
int _smi_modify_key_ever(int key)
{
    int new_key;
#ifdef WIN32
    new_key = key + 100 * GetCurrentProcessId();
#else
    new_key = key;
#endif
    
    return new_key;
}
