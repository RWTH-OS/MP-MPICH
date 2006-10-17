# set this vars as desired
DEVNAME     = ch_usock
PREFIX      = dup_
OUTDIR      = ../$(PREFIX)$(DEVNAME)
INDIR       = ../$(DEVNAME)
IGNOREFILES = *.o *.dsp *.vcproj *.kdevprj


# this files are used for gettig ste symbols
INFILE = *.o

# separation files classify types of files and threatment
SEPFILES = header.csv source.csv

default: $(OUTDIR) files.csv $(SEPFILES) symbols.csv config.inc.php
	php devdup.php
	cd $(OUTDIR); make -f Makefile setlinks default

filelist: 
	rm -f files.csv
	make files.csv

seplist: files.csv
	make $(SEPFILES)
	
files.csv: Makefile
	find $(INDIR) $(patsubst %,! -name "%",$(IGNOREFILES)) -type f -maxdepth 1 -fprintf files.csv "%f\n"

symbols.csv: 
	nm --defined-only $(INDIR)/$(INFILE) | cut -d\  -f2-3 | grep "T " | cut -d\  -f2 >symbols.csv

header.csv: files.csv
	grep "\.h" files.csv | grep -v  "\.h." >header.csv

source.csv: files.csv
	grep "\.c" files.csv | grep -v  "\.c." >source.csv

config.inc.php: Makefile
	echo "<?" >config.inc.php
	echo '$(subst #devname#,$(DEVNAME),$$DEVNAME = "#devname#";)' >>config.inc.php
	echo '$(subst #outdir#,$(OUTDIR),$$OUTDIR  = "#outdir#";)' >>config.inc.php
	echo '$(subst #indir#,$(INDIR),$$INDIR   = "#indir#";)' >>config.inc.php
	echo '$(subst #prefix#,$(PREFIX),$$PREFIX  = "#prefix#";)' >>config.inc.php
	echo "?>" >>config.inc.php

$(OUTDIR): Makefile
	if test ! -d $(OUTDIR) ; then mkdir $(OUTDIR); fi

clean:
	rm -rf $(OUTDIR)

distclean:
	make clean
	rm -f $(SEPFILES) config.inc.php files.csv symbols.csv
