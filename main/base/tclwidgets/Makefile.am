SCRIPT_FILES=@srcdir@/mwutil.tcl @srcdir@/notebook.tcl @srcdir@/tablelist.tcl @srcdir@/tablelistSortByColumn.tcl \
	@srcdir@/tablelistWidget.tcl  @srcdir@/tabnbook.tcl @srcdir@/FrameSequencer.tcl @srcdir@/FrameManager.tcl \
	@srcdir@/RingStatus.tcl @srcdir@/ChannelLabel.tcl @srcdir@/TransientLabel.tcl @srcdir@/scaleControl.tcl \
	@srcdir@/nsclspinbox.tcl \
	@srcdir@/tkutils.tcl \
	@srcdir@/warning.tcl @srcdir@/NSCLBgerror.tcl @srcdir@/bells.tcl \
	@srcdir@/icon.tcl @srcdir@/titledSpinBox.tcl @srcdir@/bindDown.tcl \
	@srcdir@/meter.tcl @srcdir@/blockimage.tcl @srcdir@/dialogwrapper.tcl \
	@srcdir@/textprompter.tcl \
	@srcdir@/selectablelist.tcl \
	@srcdir@/OutputWindow.tcl


DOCS=RingStatus.xml ScaleControl_man.xml Frames.xml transientLabel.xml

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs/widgets
	$(INSTALL_SCRIPT) $(SCRIPT_FILES) @prefix@/TclLibs/widgets
	$(INSTALL_SCRIPT) @srcdir@/ReDistribNotes @prefix@/TclLibs/widgets
	echo "pkg_mkIndex -verbose @prefix@/TclLibs/widgets *.tcl" | @TCLSH_CMD@

TCLTESTS=tclTests.tcl framemanager.test framesequencer.test channellabel.test transientlabel.test

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST = $(SCRIPT_FILES) ReDistribNotes $(TCLTESTS) $(DOCS)

check: check-TESTS

