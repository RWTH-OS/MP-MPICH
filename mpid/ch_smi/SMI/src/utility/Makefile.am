## Process this file with automake to produce Makefile.in
## $Id$

noinst_LTLIBRARIES = libutility.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include

SRCS = general.c query.c smi_time.c cpuid.c statistics.c

libutility_la_SOURCES = $(SRCS)

# TODO: Add these on problems...
#smi_time.o:	smi_time.c
#	$(GCC) $(CFLAGS_GCC) -c smi_time.c

# XXX this is more a temporary fix to compile cpuid.c w/o -fpic flag
#cpuid.o:	cpuid.c
#	$(GCC) -DHAVE_CONFIG_H -I$(SISCIDIR)/include -I$(SMIDIR)/include -I$(SMIDIR)/src -D$(CPU) -D$(OS) -c cpuid.c

