EXTRA_DIST  = 	camac.tcl	\
		wienercamac.tcl	\
		caennet.tcl cratelocator.tcl \
		caennet.xml camac.xml cratelocator.xml wienercamac.xml \
		camacutils.tcl



PACKAGE	=	@prefix@/TclLibs/sbspuretcl

install-exec-local:
	$(mkinstalldirs) $(PACKAGE)
	$(INSTALL_SCRIPT) @srcdir@/*.tcl $(PACKAGE)
	$(INSTALL_SCRIPT) @srcdir@/cratelocator.tcl @bindir@/cratelocator
	echo package ifneeded camac 1.1 [list source [file join \$$dir camac.tcl]] > $(PACKAGE)/pkgIndex.tcl
	echo package ifneeded wienercamac 1.1 [list source [file join \$$dir wienercamac.tcl]] >> $(PACKAGE)/pkgIndex.tcl
	echo package ifneeded caennet 1.5 [list source [file join \$$dir caennet.tcl]] >> $(PACKAGE)/pkgIndex.tcl
	echo package ifneeded camacutils 1.0 [list source [file join \$$dir camacutils.tcl]] >> $(PACKAGE)/pkgIndex.tcl


