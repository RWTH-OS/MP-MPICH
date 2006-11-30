## Process this file with automake to produce Makefile.in
## $Id$

SRCS =  ../src/dyn_mem/libdyn_mem.la ../src/env/libenv.la \
	../src/memory/libmemory.la \
	../src/message_passing/libmessage_passing.la \
	../src/proc_node_numbers/libproc_node_numbers.la \
	../src/proper_shutdown/libproper_shutdown.la \
	../src/regions/libregions.la ../src/startup/libstartup.la \
	../src/synchronization/libsynchronization.la \
	../src/utility/libutility.la

FULLSRCS = ../src/loop_scheduling/libloop_scheduling.la \
	   ../src/loop_splitting/libloop_splitting.la \
	   ../src/switch_consistency/libswitch_consistency.la


if FULL_SMI
lib_LTLIBRARIES = libsmi.la
libsmi_la_LIBADD = $(SRCS) $(FULLSRCS) -lstdc++
libsmi_la_SOURCES =
else
lib_LTLIBRARIES = libcsmi.la
libcsmi_la_LIBADD = $(SRCS)
libcsmi_la_SOURCES =
endif

