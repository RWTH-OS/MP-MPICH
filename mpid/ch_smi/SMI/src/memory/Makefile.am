## Process this file with automake to produce Makefile.in
## $Id$

noinst_LTLIBRARIES = libmemory.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include

SRCS = local_seg.c memcpy_base.c memcpy.c sisci_memcpy.c sci_shmem.c shmem.c \
       shseg_key.c putget.c unix_shmem.c

if CPU_X86
if !OS_DARWIN
SRCS += mmx_scimemcpy.s wc_scimemcpy.s
endif
endif

libmemory_la_SOURCES = $(SRCS)

