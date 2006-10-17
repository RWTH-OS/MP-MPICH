/* $Id$ */

/*
	In the case that several sequential SMI applications are executed on the
	same node, they all would use the same memory segments and therefore interfere
	with eachother. The reason for this is that they all use the same 'key' to
	_smi_allocate a memory segment.
	To prevent from such problems, the following function modifies a given key
	into a key that ensures that each sequential application uses different keys.
	But this only, if it really are sequential applications.
*/

/* how many shseg keys should we test? This value is the ld() of the maximum value */
#ifndef _SHSEG_KEY_H_
#define _SHSEG_KEY_H_

#define LD_MAX_SHSEG_KEY 30

int _smi_modify_key(int key);

int _smi_modify_key_ever(int key);

#endif
