install-exec-local:
	$(INSTALL_PROGRAM) @srcdir@/bcnaf.tcl       @bindir@/bcnaf
	$(INSTALL_PROGRAM) @srcdir@/cesbcnaf.tcl    @bindir@/cesbcnaf
	$(INSTALL_PROGRAM) @srcdir@/wienerbcnaf.tcl @bindir@/wienerbcnaf
	$(mkinstalldirs)   @prefix@/TclLibs
	$(mkinstalldirs)   @prefix@/TclLibs/bcnafutils
	$(INSTALL_PROGRAM) @srcdir@/bcnafutils.tcl @prefix@/TclLibs/bcnafutils
	echo pkg_mkIndex -verbose  @prefix@/TclLibs/bcnafutils bcnafutils.tcl | @TCLSH_CMD@

EXTRA_DIST = bcnaf.tcl wienerbcnaf.tcl cesbcnaf.tcl bcnaf.xml bcnafutils.tcl
