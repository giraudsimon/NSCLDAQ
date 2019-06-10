install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/XXUSBSupport
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/XXUSBSupport; done
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(prefix)/bin/`basename $$f .tcl`; done
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/XXUSBSupport *.tcl" | $(TCLSH_CMD)

TCLPACKAGES=DriverSupport.tcl \
            readoutscript.tcl \
            controlscript.tcl \
            Globals.tcl \
            xxusb.tcl \
            tclTests.tcl


TCLTESTS = driversupport.test controlscript.test readoutscript.test


check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs @prefix@/lib" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl


EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS) $(TKTESTS)

