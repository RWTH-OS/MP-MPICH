/*--------------------------------------------------------------------------*/
/*                                                                          */
/* User-level Module: SYNCMOD                                               */
/*                                                                          */
/* (c) 1998-2001 Martin Schulz, LRR-TUM                                     */
/*                                                                          */
/* Contains the HAMSTER routines for synchronization                        */
/* Standalone SISCI Version                                                 */
/*                                                                          */
/* Implementation of additional routines not present in HAMSTER             */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* routine for the distribution of identifiers */

DLLEXPORT errCode_t DLLDECL syncMod_distribute(uint master, syncMod_atomic_t *atomic)
{
  errCode_t err=OK_SIMPLESYNC,err2;
  
  if (locNodeNum==master)
    {
      err=atomic_setAtomicDirect(ATOMIC_SCOPE,*atomic);
    }

  err2=syncMod_fixedBarrier(ATOMIC_BARRIER,locNodeCount);
  if (err!=OK_SIMPLESYNC)
    return err;
  if (err2!=OK_SIMPLESYNC)
    return err2;

  if (locNodeNum!=master)
    {
      err=atomic_getAtomicDirect(ATOMIC_SCOPE,atomic);
    }

  err2=syncMod_fixedBarrier(ATOMIC_BARRIER,locNodeCount);
  if (err!=OK_SIMPLESYNC)
    return err;
  if (err2!=OK_SIMPLESYNC)
    return err2;

}


/*--------------------------------------------------------------------------*/
/* The End. */
