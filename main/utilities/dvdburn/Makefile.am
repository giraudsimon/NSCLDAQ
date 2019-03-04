TCLPACKAGES=burnpackage.tcl TopGUi.ui.tcl dvdburnoutput.ui.tcl
TCLAPPS=dvdburn burngui

install-exec-local:
	$(mkinstalldirs) @prefix@/bin
	$(mkinstalldirs) @prefix@/TclLibs/dvdburn
	for f in $(TCLPACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/dvdburn; done
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f      @prefix@/bin; done
	echo "pkg_mkIndex @prefix@/TclLibs/dvdburn *.tcl" > makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl


EXTRA_DIST = $(TCLAPPS) $(TCLPACKAGES) dvdburn.xml  burnpackage.xml