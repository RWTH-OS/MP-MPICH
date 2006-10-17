## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.1.2.1 2004/12/20 17:26:39 tobias Exp $

SUBDIRS = dyn_mem env memory message_passing proc_node_numbers \
	  proper_shutdown regions startup synchronization utility

if FULL_SMI
SUBDIRS += loop_scheduling loop_splitting switch_consistency
endif

