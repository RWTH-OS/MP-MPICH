## Process this file with automake to produce Makefile.in
## $Id$

SUBDIRS = memcpy_bench poisson sort test

install-data-hook:
	-rm -f $(DESTDIR)$(docdir)/examples
	$(LN_S) $(pkglibdir)/examples $(DESTDIR)$(docdir)/examples

uninstall-hook:
	-rm -f $(DESTDIR)$(docdir)/examples
