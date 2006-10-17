/* $Id$ */

#include "dyn_mem.h"
#include "env/smidebug.h"


/*
  |  Diese Funktion initialisiert die Speicherverwaltung fuer das  |
  |  uebergegebene Speichersegment. Zunaechst werden die ersten    |
  |  4 Kbytes fuer die Speicherverwaltung reserviert. Das erste    |
  |  Element enthaelt die Wurzel des Verwaltungsbaumes und das     |
  |  zweite den Kopf der Liste mit den freien Verwaltungselemen-   |
  |  ten. Erst danach wird der Verwaltungsbaum aufgebaut. Der      |
  |  erste belegte Speicherplatz ist die Baumverwaltung selber.    |
  |  Falls die Initialisierung des Speichersegmentes ohne Probleme |
  |  verlaufen ist, wird der Wert 0 uebergeben, ansonsten ein ent- |
  |  sprechender Fehlercode.                                       |
*/
int _smi_dynmem_init (void *sgmt_adr, size_t sgmt_sz)
{
    DSECTION ("_smi_dynmem_init");
	smi_memmgr_t *base_node, *free_list;
	size_t tmp;
   
	base_node = (smi_memmgr_t *) sgmt_adr;
   
	/* Fix, then check segment size. The used buddy-technique can only
	   handle power-of-2 sizes. */
	tmp = 1;
	while (2*tmp <= sgmt_sz)
		tmp <<= 1;
	sgmt_sz = tmp; 
	if (sgmt_sz <= 2*FREELIST_SZ) {
		DPROBLEMI ("dynamic memory region is too small: min. size is", 2*FREELIST_SZ);
		return 1;
	}

	/* Create free_list. */
	free_list = base_node + 1;
	free_list->b_size = 0;
	free_list->sb_l = NULL;
	free_list = _smi_free_list_init(base_node+2, free_list);
 
	/* Create the base node of the management system. */
	base_node->b_size = sgmt_sz;
	base_node->b_addr = (char * )base_node;
	base_node->sb_l = NULL;
	base_node->sb_r = NULL;
	base_node->sb_avail = sgmt_sz;
	base_node->mem_avail = sgmt_sz;

	/* Mark base_node and free_list as allocated. */
	_smi_dynmem_memtree(base_node, free_list, FREELIST_SZ);     

	DNOTICEI("Set up MMU for a region; effective size is", base_node->b_size);
	DNOTICEI("Available memory for MMU is", base_node->sb_avail);

	return 0;
}


smi_memmgr_t *_smi_free_list_init (smi_memmgr_t *free_node, smi_memmgr_t *free_list)
{
    REMDSECTION("_smi_free_list_init");
	int i, cnt;

	DSECTENTRYPOINT;

	/* Increase available management slots. 'b_size' is "abused" for this - it
	   is the number of available (free) block allocation entries. */
	cnt = FREELIST_SZ/sizeof(smi_memmgr_t);
	if (free_list->sb_l == NULL) 
		cnt -= 2;
	free_list->b_size += cnt;

	DNOTICEI("free_list->b_size",free_list->b_size);
	DNOTICEI("cnt",cnt);

	/* Insert all availabe block allocation entries at the front. */
	for (i = 0; i < cnt; i++)	{
		DNOTICEP("Inserting node to free_list:",free_node);
		free_node->sb_l = free_list->sb_l;
		free_list->sb_l = free_node;
		DNOTICEP("free_node->sb_l",free_node->sb_l);
		free_node++; 
	}

	DSECTLEAVE;
	return free_list;
}













/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */



