/* $Id$ */

#include <pthread.h>

#include "env/smidebug.h"
#include "env/general_definitions.h"
#include "sendrecv.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int      bInitialized = FALSE;
static char   **ppAlocatedAreas;
static volatile char **volatile ppDest;
static volatile char **volatile ppRecv;

pthread_mutex_t mpMutex;
static int     *pLocked;

#define BACKOFF_US 1000

static smi_error_t _smi_locksend(int dest)
{
    smi_error_t         RetVal = SMI_SUCCESS;

    SMI_LOCK(&mpMutex);

    if (pLocked[dest] == FALSE) {
	pLocked[dest] = TRUE;
    } else {
	RetVal = SMI_ERR_PENDING;
    }

    SMI_UNLOCK(&mpMutex);

    return (RetVal);
}

static smi_error_t _smi_releasesend(int dest)
{
    smi_error_t         RetVal = SMI_SUCCESS;;

    SMI_LOCK(&mpMutex);

    if (pLocked[dest] == FALSE)
	RetVal = SMI_ERR_NOTPOSTED;
    pLocked[dest] = FALSE;

    SMI_UNLOCK(&mpMutex);

    return (RetVal);
}

static int _smi_checksend(int dest)
{
    int             RetVal;

    SMI_LOCK(&mpMutex);
    RetVal = pLocked[dest];
    SMI_UNLOCK(&mpMutex);

    return (RetVal);
}

static smi_error_t _smi_reset_mp(void)
{
    int             i;
    smi_mp_packet_t Packet;

    SMI_LOCK(&mpMutex);

    memset(&Packet, 0, sizeof(Packet));
    Packet.tag = _smi_my_proc_rank;
    for (i = 0; i < _smi_nbr_procs; i++) {
	memcpy((void *)ppRecv[i], &Packet, sizeof(Packet));
	pLocked[i] = FALSE;
    }

    SMI_UNLOCK(&mpMutex);

    return (SMI_SUCCESS);
}

smi_error_t _smi_init_mp()
{
    DSECTION("_smi_init_mp");
    int             i, j;
    smi_error_t         tError;
    int             iProcsOnNode;

    DSECTENTRYPOINT;

    if (bInitialized == TRUE) {
	DSECTLEAVE
	    return (SMI_SUCCESS);
    }
    DNOTICE("allocating memory for areapointers");
    ALLOCATE(ppAlocatedAreas, char **, _smi_nbr_machines * sizeof(char *));
    ALLOCATE(ppDest, volatile char **, _smi_nbr_procs * sizeof(char *));
    ALLOCATE(ppRecv, volatile char **, _smi_nbr_procs * sizeof(char *));
    ALLOCATE(pLocked, int *, sizeof(int) * _smi_nbr_procs);

    DNOTICE("allocating memoryareas for messagepassing on internal segments");
    for (i = 0; i < _smi_nbr_machines; i++) {
	iProcsOnNode = _smi_last_proc_on_node(i) - _smi_first_proc_on_node(i) + 1;
	tError = SMI_Cmalloc(iProcsOnNode * SMI_MP_MAXSIZE * _smi_nbr_procs,
			     _smi_first_proc_on_node(i) | INTERNAL, (void **)&ppAlocatedAreas[i]);
	DNOTICEP("ppAlocatedArea =", ppAlocatedAreas[i]);
	ASSERT_R(tError == SMI_SUCCESS, "SMI_Imalloc failed", SMI_ERR_OTHER);
	ASSERT_R(((size_t)(ppAlocatedAreas[i]) & 63) == 0, "Adress is not 64 byte aligned", SMI_ERR_OTHER);
	for (j = 0; j < iProcsOnNode; j++) {
	    /* addresses for sending */
	    ppDest[j + _smi_first_proc_on_node(i)]
		= ppAlocatedAreas[i]	/* baseaddress for this node */
		+(j * SMI_MP_MAXSIZE * _smi_nbr_procs)	/* offset for this process */
		+(_smi_my_proc_rank * SMI_MP_MAXSIZE);	/* offset for the sending process */

	    /*  base address for receiving */
	    if (_smi_my_proc_rank == _smi_first_proc_on_node(i) + j)
		ppRecv[0] = ppAlocatedAreas[i] + (j * SMI_MP_MAXSIZE * _smi_nbr_procs);
	}
    }

    /* offsets for receiving */
    for (i = 1; i < _smi_nbr_procs; i++)
	ppRecv[i] = ppRecv[i - 1] + SMI_MP_MAXSIZE;

    SMI_INIT_LOCK(&mpMutex);

    _smi_reset_mp();

    bInitialized = TRUE;

    DSECTLEAVE;
    return (SMI_SUCCESS);
}

smi_error_t _smi_finalize_mp()
{
    DSECTION("_smi_finalize_mp");
    int             i;
    smi_error_t         tError;

    DSECTENTRYPOINT;

    ASSERT_R((bInitialized == TRUE), "calling finalize without initialization", SMI_ERR_NOINIT);

    SMI_DESTROY_LOCK(&mpMutex);

    for (i = 0; i < _smi_nbr_machines; i++) {
	tError = SMI_Cfree(ppAlocatedAreas[i]);
	ASSERT_R((tError == SMI_SUCCESS), "SMI_Cfree failed", SMI_ERR_OTHER);
    }

    free((void*)ppAlocatedAreas);
    free((void*)ppDest);
    free((void*)ppRecv);
    free((void*)pLocked);

    DSECTLEAVE
	return (SMI_SUCCESS);
}

smi_error_t SMI_Send(void *buf, int count, int dest)
{
    DSECTION("SMI_Send");
    smi_mp_packet_t Packet;

    DSECTENTRYPOINT;

    ASSERT_R((bInitialized == TRUE), "calling without initialization", SMI_ERR_NOINIT);
    ASSERT_R((dest != _smi_my_proc_rank), "cannot send to myself", SMI_ERR_PARAM);

    if ((count < 0) || (count > SMI_MP_MAXDATA)) {
	DSECTLEAVE; return (SMI_ERR_PARAM);
    }
    if (_smi_locksend(dest) == SMI_ERR_PENDING) {
	DSECTLEAVE; return (SMI_ERR_PENDING);
    }

    memcpy(Packet.data, buf, count);
    Packet.tag = _smi_my_proc_rank;
    Packet.size = count;
    
    DNOTICEP("sending to address:", ppDest[dest]);
    do {
	memcpy((void *)ppDest[dest], &Packet, sizeof(Packet));
	DNOTICEI("tag:", ((smi_mp_packet_t volatile *) ppDest[dest])->tag);
    } while (SMI_Check_transfer_proc(dest, 0) != SMI_SUCCESS);

    /* wait for acknowledge from receiver */
    SMI_Flush_read (dest);
    while (((smi_mp_packet_t *)ppDest[dest])->tag != dest) {
	SMI_Flush_read (dest);
	usleep(BACKOFF_US);
    }

    _smi_releasesend(dest);

    DSECTLEAVE; return (SMI_SUCCESS);
}

smi_error_t SMI_Recv(void *buf, int count, int src)
{
    DSECTION("SMI_Recv");
    smi_mp_packet_t Packet;

    DSECTENTRYPOINT;

    ASSERT_R((bInitialized == TRUE), "calling without initialization", SMI_ERR_NOINIT);
    ASSERT_R((src != _smi_my_proc_rank), "cannot recv from myself", SMI_ERR_PARAM);

    if ((count < 0) || (count > SMI_MP_MAXDATA)) {
	DSECTLEAVE; return (SMI_ERR_PARAM);
    }

    DNOTICEP("receiving from address:", ppRecv[src]);
    /* wait for incoming message */
    while (((volatile smi_mp_packet_t *) (ppRecv[src]))->tag != src) {
	SMI_Flush_read (src);
	usleep(BACKOFF_US);
    }

    memcpy(&Packet, (void *)ppRecv[src], sizeof(Packet));
    ASSERT_R((Packet.size == count), "Size does not match", SMI_ERR_PARAM);
    memcpy(buf, Packet.data, count);

    /* acknowledge the arrival of the message */
    Packet.tag = _smi_my_proc_rank;
    memcpy((void *)ppRecv[src], &Packet, sizeof(Packet));

    DSECTLEAVE;
    return (SMI_SUCCESS);
}

smi_error_t SMI_Isend(void *buf, int count, int dest)
{
    DSECTION("SMI_ISend");
    smi_mp_packet_t Packet;

    DSECTENTRYPOINT;

    ASSERT_R((bInitialized == TRUE), "calling without initialization", SMI_ERR_NOINIT);
    ASSERT_R((dest != _smi_my_proc_rank), "cannot send to myself", SMI_ERR_PARAM);

    if ((count < 0) || (count > SMI_MP_MAXDATA)) {
	DSECTLEAVE; return (SMI_ERR_PARAM);
    }
    if (_smi_locksend(dest) == SMI_ERR_PENDING) {
	DSECTLEAVE; return (SMI_ERR_PENDING);
    }

    memcpy(Packet.data, buf, count);
    Packet.tag = _smi_my_proc_rank;
    Packet.size = count;

    do {
	memcpy((void *)ppDest[dest], &Packet, sizeof(Packet));
    } while (SMI_Check_transfer_proc(dest, 0) != SMI_SUCCESS);

    DSECTLEAVE; return (SMI_SUCCESS);
}

smi_error_t SMI_Send_wait(int dest)
{
    DSECTION("SMI_Send_wait");
    DSECTENTRYPOINT;

    ASSERT_R((bInitialized == TRUE), "calling without initialization", SMI_ERR_NOINIT);
    ASSERT_R((dest != _smi_my_proc_rank), "cannot send to myself", SMI_ERR_PARAM);

    if (_smi_checksend(dest) == FALSE) {
	DSECTLEAVE; return (SMI_ERR_NOTPOSTED);
    }

    SMI_Flush_read(dest);
    while (((smi_mp_packet_t *)ppDest[dest])->tag != dest) {
	usleep(BACKOFF_US);
	SMI_Flush_read(dest);
    }

    _smi_releasesend(dest);

    DSECTLEAVE;	return (SMI_SUCCESS);
}

smi_error_t SMI_Sendrecv(void *send_buf, void *recv_buf, int count, int dest)
{
    DSECTION("SMI_Sendrecv");
    smi_error_t         tError;

    DSECTENTRYPOINT;

    ASSERT_R((bInitialized == TRUE), "calling without initialization", SMI_ERR_NOINIT);
    ASSERT_R((dest != _smi_my_proc_rank), "cannot send/recv myself", SMI_ERR_PARAM);

    tError = SMI_Isend(send_buf, count, dest);
    ASSERT_R((tError == SMI_SUCCESS), "SMI_Isend failed", tError);
    tError = SMI_Recv(recv_buf, count, dest);
    ASSERT_R((tError == SMI_SUCCESS), "SMI_Recv failed", tError);
    tError = SMI_Send_wait(dest);
    ASSERT_R((tError == SMI_SUCCESS), "SMI_Send_wait failed", tError);

    DSECTLEAVE;	return (SMI_SUCCESS);
}
