install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs/s800
	$(INSTALL_SCRIPT) @srcdir@/s800rdocallouts.tcl @prefix@/TclLibs/s800
	$(INSTALL_SCRIPT) @srcdir@/s800rctl.tcl        @prefix@/TclLibs/s800
	$(INSTALL_SCRIPT) @srcdir@/dummyrdo            $(bindir)
	echo pkg_mkIndex @prefix@/TclLibs/s800 "*.tcl" | tclsh


EXTRA_DIST=s800rdocallouts.tcl s800rctl.tcl dummyrdo s800docs.xml tclTests.tcl s800rctl.test \
					fakeControlServer.tcl


check-TESTS : 
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

check: check-TESTS
