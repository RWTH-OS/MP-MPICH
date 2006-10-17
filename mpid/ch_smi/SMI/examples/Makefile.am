## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.1.2.1 2004/12/20 17:26:35 tobias Exp $

SUBDIRS = memcpy_bench poisson sort test

install-data-hook:
	-rm -f $(DESTDIR)$(docdir)/examples
	$(LN_S) $(pkglibdir)/examples $(DESTDIR)$(docdir)/examples

uninstall-hook:
	-rm -f $(DESTDIR)$(docdir)/examples
