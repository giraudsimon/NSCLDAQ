bin_PROGRAMS = qtscope.bin
include_HEADERS = CPyHelper.h

lib_LTLIBRARIES = libPixieUtilities.la

qtscope_bin_SOURCES = qtscope.cpp
qtscope_bin_CXXFLAGS = @PYTHON3_CFLAGS@ \
	@PIXIE_CPPFLAGS@ \
	-DPREFIX=\"@prefix@\"
qtscope_bin_LDFLAGS = @PYTHON3_LIBS@

libPixieUtilities_la_SOURCES = \
	CPixieDSPUtilities.cpp CPixieDSPUtilities.h \
	CPixieRunUtilities.cpp CPixieRunUtilities.h \
	CPixieSystemUtilities.cpp CPixieSystemUtilities.h \
	CPixieTraceUtilities.cpp CPixieTraceUtilities.h \
	CDataGenerator.cpp CDataGenerator.h
libPixieUtilities_la_CPPFLAGS = \
	-I@top_srcdir@ \
	-I@top_srcdir@/ddas/configuration \
	-I@top_srcdir@/ddas/booter \
	@PIXIE_CPPFLAGS@ \
	-DFIRMWARE_FILE=\"@prefix@/share/ddasreadout/DDASFirmwareVersions.txt\"
libPixieUtilities_la_LIBADD = \
	@top_builddir@/ddas/configuration/libConfiguration.la \
        @top_builddir@/ddas/booter/libSystemBooter.la \
	@PIXIE_LDFLAGS@

FIGURES = @srcdir@/figures/timing_diagram.png

install-data-hook:
	$(mkinstalldirs) @prefix@/ddas/qtscope
	$(mkinstalldirs) @prefix@/ddas/qtscope/figures
	for f in $(FIGURES) ; do $(INSTALL_DATA) $$f @prefix@/ddas/qtscope/figures/. ; done
	$(INSTALL_DATA) @srcdir@/*.py @prefix@/ddas/qtscope

install-exec-hook:
	$(mkinstalldirs) @prefix@/bin
	$(INSTALL_PROGRAM) qtscope @prefix@/bin

EXTRA_DIST = acquisition_toolbar.py adc_trace.py analog_signal.py baseline.py \
	cfd.py chan_dsp_gui.py chan_dsp_layout.py chan_dsp_manager.py \
	chan_dsp_widget.py colors.py converters.py crate_id.py csra.py csrb.py \
	dsp_manager.py dsp_toolbar.py energy_filter.py fit_exp_creator.py \
	fit_factory.py fit_gauss_creator.py fit_gauss_p1_creator.py \
	fit_gauss_p2_creator.py fit_panel.py gui.py main.py mod_dsp_gui.py \
	mod_dsp_layout.py mult_coincidence.py pixie_utilities.py plot.py \
	plot_toolbar.py qdclen.py run_type.py system_toolbar.py tau.py \
	timing_control.py trace_analyzer.py trigconfig0.py trigger_filter.py \
	widget_factory.py worker.py xia_constants.py
