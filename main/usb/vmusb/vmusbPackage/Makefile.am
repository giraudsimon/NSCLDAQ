EXTRA_DIST=vmusbPackage.tcl vmusbvme.xml


install-exec-local: 
	$(mkinstalldirs) @prefix@/TclLibs/vmusbtcl
	install @srcdir@/vmusbPackage.tcl @prefix@/TclLibs/vmusbtcl
	echo "package ifneeded vmusbvme 1.0 [list source [file join \$$dir vmusbPackage.tcl]]" > @prefix@/TclLibs/vmusbtcl/pkgIndex.tcl
 

