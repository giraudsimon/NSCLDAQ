PKGFILES=@srcdir@/marker.tcl @srcdir@/pkgIndex.tcl @srcdir@/series.tcl \
	@srcdir@/xyplot.tcl @srcdir@/series.test

DOCS=plotcontainer.xml

DESTDIR = @prefix@/TclLibs/plotcontainer

TEST_FILES=tclTests.tcl series.test

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)
	$(INSTALL_SCRIPT) $(PKGFILES) $(DESTDIR) 

check-TESTS:
	(cd @srcdir@ && @TCLSH@ tclTests.tcl)

EXTRA_DIST=$(PKGFILES) $(DOCS) $(TEST_FILES)

check: check-TESTS
