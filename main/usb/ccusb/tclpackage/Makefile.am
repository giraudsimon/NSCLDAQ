EXTRA_DIST  = 	ccusbcamac.tcl ccusbcamac.xml


TCLPACKAGES = ccusbcamac.tcl 

PACKAGE	=	@prefix@/TclLibs/ccusbpuretcl

install-exec-local:
	$(mkinstalldirs) $(PACKAGE)
	for f in $(TCLPACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(PACKAGE) ; done
	echo package ifneeded ccusbcamac 1.0 [list source [file join \$$dir ccusbcamac.tcl]] > $(PACKAGE)/pkgIndex.tcl

