/* $Id$ */

#ifndef _STORE_BARRIER_H_
#define _STORE_BARRIER_H_

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


    /*
     * internal prototypes 
     */


/***************************************************************************/
/* To implement software synchronization algorithms correctly on system    */
/* with a weaker memory consistency than sequential consistency, one needs */
/* operations, the so-called 'store'barriers' that guarantee that all      */
/* write-operations of a specific processor have been finished (i.e. took  */
/* place in memory), if the next operation after this store-barrier is     */
/* executed. The following functions enforce this in the local case as     */
/* well as in the remote case.                                             */
/***************************************************************************/


/***************************************************************************/
/* _smi_allocate ALLSTREAMS_SIZE Byte of remote memory (where is not important; */
/* this is enough for the Intel systems as well as the sun systems))       */
/* to do so, search for a process on a remote node                         */
/* the first address of remote memory is ensured to lay in the first byte  */
/* of the first sci buffer                                                 */
/***************************************************************************/
void _smi_init_load_store_barriers(void);

/***************************************************************************/
/* System-wide store-barrier                                               */
/***************************************************************************/
void _smi_store_barrier(void);


/***************************************************************************/
/* SMP-wide store-barrier                                                  */
/***************************************************************************/
void _smi_local_store_barrier(void);

/***************************************************************************/
/* System-wide load-barrier                                                */
/***************************************************************************/
int _smi_load_barrier(void);

/***************************************************************************/
/* System-wide load-barrier for a small address range                      */
/* the last parameter states on which proc the data resides that shall     */
/* be flushed; '-1' will flush data locally and system wide                */
/***************************************************************************/
int _smi_range_store_barrier(volatile void* start, unsigned int size, int home_of_data);

/***************************************************************************/
/* System-wide load-barrier for a small address range                      */
/* the last parameter states on which proc the data resides that shall     */
/* be freshly loaded; '-1' will flush data locally and system wide         */
/***************************************************************************/
int _smi_range_load_barrier(volatile void* start, unsigned int size, int home_of_data);



#ifdef __cplusplus
}
#endif


#endif
