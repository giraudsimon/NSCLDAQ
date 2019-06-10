PKGFILES=@srcdir@/marker.tcl @srcdir@/pkgIndex.tcl @srcdir@/series.tcl \
	@srcdir@/xyplot.tcl @srcdir@/series.test

DOCS=plotcontainer.xml

DESTDIR = @prefix@/TclLibs/plotcontainer

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)
	$(INSTALL_SCRIPT) $(PKGFILES) $(DESTDIR) 

check-TESTS:
	(cd @srcdir@; tclsh series.test)

EXTRA_DIST=$(PKGFILES) $(DOCS)
