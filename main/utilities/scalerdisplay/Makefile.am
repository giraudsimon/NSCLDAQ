install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(mkinstalldirs) @prefix@/TclLibs/ScalerDisplay
	for f in $(TCLPACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/ScalerDisplay; done
	$(INSTALL_SCRIPT) @builddir@/ScalerDisplay @srcdir@/ScalerDisplay.tcl $(bindir)


TCLPACKAGES=scaler.tcl 
TCLAPPS=ScalerDisplay ScalerDisplay.tcl

EXTRA_DIST = ScalerDisplay.tcl scaler.tcl \
 ScalerDisplay.in scalerdisplay.xml

