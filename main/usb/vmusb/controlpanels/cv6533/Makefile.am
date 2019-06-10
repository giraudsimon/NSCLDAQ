PACKAGES=channel.tcl channelParams.tcl v6533.tcl
APP=v6533panel.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/v6533
	echo pkg_mkIndex -verbose @prefix@/TclLibs/v6533 "*.tcl" | tclsh	
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/v6533; done
	for f in $(APP); do $(INSTALL_SCRIPT) @srcdir@/$$f  @bindir@; done

EXTRA_DIST=$(APP) $(PACKAGES)