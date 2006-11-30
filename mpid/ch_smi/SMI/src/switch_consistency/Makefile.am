## Process this file with automake to produce Makefile.in
## $Id$

noinst_LTLIBRARIES = libswitch_consistency.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include

SRCS = combine_add.c copy.c copy_every_local.c copy_gl_dist_local.c \
       ensure_consistency.c init_switching.c switch_to_replication.c \
       switch_to_sharing.c

# FIXME: These files contain calls to _smi_copy_from_to_double which has gone
# missing since at lest 1999...
# switch_to_replication_fast.c switch_to_sharing_fast.c

libswitch_consistency_la_SOURCES = $(SRCS)

