## Process this file with automake to produce Makefile.in
## $Id$

exampleexecdir = ${pkglibdir}/examples/test

exampleexec_PROGRAMS = helloworld signal_many signal sync regions reglimits \
	memperf put alltoall mutex dma registering fragmented region_cb \
	regalloc

if FULL_SMI
exampleexec_PROGRAMS += replicate
endif

if CPU_X86
exampleexec_PROGRAMS += memcpy strided
endif

AM_CPPFLAGS = -I$(top_srcdir)/include
LDADD = ../../lib/lib$(LIBNAME).la

alltoall_SOURCES = alltoall.c
dma_SOURCES = dma.c
fragmented_SOURCES = fragmented.c
helloworld_SOURCES = helloworld.c
memcpy_SOURCES = memcpy.c
memperf_SOURCES = memperf.c
mutex_SOURCES = mutex.c
put_SOURCES = put.c
regalloc_SOURCES = regalloc.c
region_cb_SOURCES = region_cb.c
regions_SOURCES = regions.c
registering_SOURCES = registering.c
reglimits_SOURCES = reglimits.c
replicate_SOURCES = replicate.c
signal_SOURCES = signal.c
signal_many_SOURCES = signal_many.c
strided_SOURCES = strided.c
sync_SOURCES = sync.c

