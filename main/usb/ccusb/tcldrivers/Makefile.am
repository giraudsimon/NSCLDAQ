DRIVERDest=@prefix@/TclLibs/ccusbdrivers

TCLDRIVERS=ccusb.tcl CCUSBDriverSupport.tcl \
					 ACAENC671.tcl ACAENC808.tcl \
					 ALeCroy4448.tcl ATrigger2367.tcl \
					 APhillips71xx.tcl ALeCroy4300B.tcl \
					 AOrtecCCF8200.tcl
					 
           

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
TCLTESTS = acaen671.test \
					 acaen808.test \
					 ccusbdriver.test \
					 lecroy4300b.test \
					 lecroy4448.test \
					 mockccusb.test \
					 phillips71xx.test \
					 trigger2367.test \
					 tclTests.tcl  \
					 MockCCUSB.tcl


check-TESTS:
	echo "pkg_mkIndex -verbose @srcdir@ MockCCUSB.tcl" | $(TCLSH_CMD)
	TCLLIBPATH="@prefix@/TclLibs @libdir@ @srcdir@" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) $(TCLTESTS)
#$(CCUSBTCLTESTS) 

