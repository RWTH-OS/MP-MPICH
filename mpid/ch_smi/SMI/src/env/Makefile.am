## Process this file with automake to produce Makefile.in
## $Id$

noinst_LTLIBRARIES = libenv.la
EXTRA_LTLIBRARIES = libenvfort.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include

SRCS = error_count.c general_definitions.c page_size.c redirect_io.c safety.c \
       setup.c smi_finalize.c smi_init.c smi_fifo.c

libenv_la_SOURCES = $(SRCS)

if FORTRAN
libenv_la_LIBADD = libenvfort.la
endif

libenvfort_la_SOURCES = fortran_binding.c getarg.f

