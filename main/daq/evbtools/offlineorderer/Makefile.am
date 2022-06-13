
# don't forget the figures for the documentation
DIST_SUBDIRS = figures

TCLDESTDIR = @prefix@/TclLibs/OfflineEVB

install-exec-local:
	$(mkinstalldirs) @prefix@/bin
	$(mkinstalldirs) $(TCLDESTDIR)
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(TCLDESTDIR); done
	install @srcdir@/offlineorderer.tcl @prefix@/bin/offlineorderer
	echo "pkg_mkIndex -verbose $(TCLDESTDIR) *.tcl" | $(TCLSH_CMD)


TCLPACKAGES=InputPipeline.tcl HoistPipeline.tcl \
            OutputPipeline.tcl \
            JobBuilderUI.tcl JobConfigUI.tcl \
            ConfigurationErrorUI.tcl \
            MissingSourceUI.tcl \
            JobProcessor.tcl RunProcessor.tcl \
            RunStatusUI.tcl  \
            FileListWidget.tcl \
            OfflineOrdererUI.tcl \
            GlobalConfigUI.tcl \
						EVBConfigure.tcl \
            Job.tcl

TCLAPPS=offlineorderer.tcl

TCLTESTS=tclTests.tcl filelistwidget.test hoistpipeline.test inputpipeline.test \
				 jobbuilderui.test jobprocessor.test job.test outputpipeline.test \
				 runprocessor.test runstatusui.test globalconfig.test

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST = $(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS)


check: check-TESTS
