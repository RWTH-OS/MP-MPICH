## Process this file with automake to produce Makefile.in
## $Id$

noinst_LTLIBRARIES = libregions.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include

SRCS = address_to_region.c connect_shreg.c create_shreg.c free_shreg.c \
       internal_regions.c memtree.c idstack.c print_regions.c \
       region_layout.c segment_address.c

libregions_la_SOURCES = $(SRCS)

