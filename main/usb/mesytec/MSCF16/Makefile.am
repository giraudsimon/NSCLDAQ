
SUBDIRS=figures

DRIVERDest = @prefix@/TclLibs/mesytec

TCLDRIVERS = MSCF16USB.tcl MSCF16Gui.tcl MSCF16GuiApp.tcl \
						 MSCF16Memorizer.tcl MSCF16CommandLogger.tcl \
						 MSCF16ScriptHeaderGenerator.tcl \
						 MSCF16StateSaver.tcl MSCF16FileLoader.tcl
TCLAPPS = MSCF16Control.tcl

DOCS = mscf16control.xml

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/bin/`basename $$f .tcl`  && \
		chmod a+x @prefix@/bin/`basename $$f .tcl` ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
TCLTESTS=tclTests.tcl mscf16usb.test mscf16memorizer.test \
				 mscf16commandlogger.test mscf16scriptheadergenerator.test \
				 mscf16statesaver.test mscf16fileloader.test

LOG_COMPILER = @TCLSH_CMD@ tclTests.tcl 

AM_TESTS_ENVIRONMENT = TCLLIBPATH="@prefix@/TclLibs" HERE=@srcdir@ tcl=@TCLSH_CMD@ ; \
											 export TCLLIBPATH; \
											 export HERE; \
											 export tcl
installcheck-local:
	TCLLIBPATH="@prefix@/TclLibs" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) $(TCLTESTS) $(TCLAPPS) $(DOCS)
