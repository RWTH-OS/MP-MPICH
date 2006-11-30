## Process this file with automake to produce Makefile.in
## $Id$

noinst_LTLIBRARIES = libproc_node_numbers.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include

SRCS = first_proc_on_node.c node_rank.c node_size.c node_name.c proc_rank.c \
       proc_size.c proc_to_node.c

libproc_node_numbers_la_SOURCES = $(SRCS)

