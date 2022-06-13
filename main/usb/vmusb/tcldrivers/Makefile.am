DRIVERDest=@prefix@/TclLibs/vmusbdrivers

TCLDRIVERS=sis330xdriver.tcl sis3300.tcl  VMUSBDriverSupport.tcl vmusb.tcl \
           AXLM72.tcl APpacXLM72.tcl ACrdcXLM72.tcl AXLM72Scaler.tcl \
	   ACAENV288.tcl ACAENN568B.tcl AGD16XLM72.tcl AXLM72Generic.tcl \
	ALevel3XLM72Readout.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
TCLTESTS=tclTests.tcl vmusbdriversupport.test manXLM72Scaler.tcl ppacxlm72.test crdcxlm72.test gd16xlm72.test


check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs @prefix@/lib" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) $(TCLTESTS)
