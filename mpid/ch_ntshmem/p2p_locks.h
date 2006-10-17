/* p2p locks.  This file includes code for picking a strategy,
 * whether generic (e.g., the SysV SEMOPS) or machine specific.
 * First we choose a method, and then, in a second step, generate the
 * appropriate declarations.
 */

/* Choose method for providing locks.  The decision will be made by
 * combining preferences (defined here) with available capabilities (should be,
 * and for the most part are, defined by configure as HAVE_, al).  Some of the
 * HAVE_'s may be defined in p2p_special.h, at least temporarily.
 */



#include <wtypes.h>
#include <winbase.h>

typedef HANDLE p2p_lock_t;

/* The handles of the mutexes must be inheritable so we 
   need the security attributes here */
extern SECURITY_ATTRIBUTES attr;

#    define p2p_lock_init(l)  *l=CreateMutex(&attr,FALSE,NULL)
#    define p2p_lock(l)       WaitForSingleObject(*l,INFINITE)
#    define p2p_unlock(l)     ReleaseMutex(*l)
#    define p2p_lock_name     "anonymous mutex"




