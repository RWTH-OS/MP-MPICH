## Process this file with automake to produce Makefile.in
## $Id: Makefile.am 3002 2004-12-20 17:26:47Z tobias $

lib_LTLIBRARIES = libnt2unix.la

AM_CXXFLAGS = -I$(top_srcdir)/include

libnt2unix_la_SOURCES = ../src/threadsync.cc ../src/handle.cc \
	../src/virtualmemory.cc ../src/general.cc ../src/environment.cc \
	../src/exception.cc ../src/process.cc ../src/filemap.cc \
	../src/threads.cc ../src/winsockapi.cc ../src/winsock2api.cc \
	../src/iphlpapi.cc ../src/mydebug.cc

# CXXFLAGS = -finline-functions -fno-rtti

