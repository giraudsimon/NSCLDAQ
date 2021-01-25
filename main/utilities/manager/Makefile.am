program_scripts=startServer.tcl
library_scripts=

EXTRADIST=$program_scripts $library_scripts

install-exec-local:
	for f in $(program_scripts); do $(INSTALL_PROGRAM) @srcdir@/$$f @bindir@; done
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/manager
	for f in $(library_scripts); do $(INSTALL_DATA) @srcdir@/$$f @prefix@/TclLibs/manager; done

