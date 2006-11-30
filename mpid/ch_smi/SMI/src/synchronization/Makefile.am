## Process this file with automake to produce Makefile.in
## $Id$

noinst_LTLIBRARIES = libsynchronization.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include

SRCS = smibarrier.c mutex.c progress.c signalization.c store_barrier.c \
       sync_finalize.c syncmod.c sync_init.c

libsynchronization_la_SOURCES = $(SRCS)

