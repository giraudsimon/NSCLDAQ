TCLPACKAGES=sequencerGui.tcl controller.tcl ReadoutControl.tcl  pkgIndex.tcl 
EXAMPLEFILES=ReadoutCallouts.tcl sequencerActions.tcl 

EXTRA_DIST=$(TCLPACKAGES) $(EXAMPLEFILES) sequencer.xml


install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs/sequencer
	$(mkinstalldirs) @prefix@/examples/sequencer
	for f in $(TCLPACKAGES) ; do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/sequencer; done
	for f in $(EXAMPLEFILES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/examples/sequencer; done


