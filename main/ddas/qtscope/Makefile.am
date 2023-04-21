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

install-exec-hook:
	$(mkinstalldirs) @prefix@/bin
	$(mkinstalldirs) @prefix@/ddas/qtscope
	$(INSTALL_PROGRAM) qtscope @prefix@/bin
	$(INSTALL_DATA) @srcdir@/*.py @prefix@/ddas/qtscope

EXTRA_DIST = acquisition_toolbar.py chan_dsp_widget.py energy_filter.py \
	gui.py plot_toolbar.py trace.py analog_signal.py colors.py \
	fit_exp_creator.py main.py qdclen.py trigconfig0.py baseline.py \
	crate_id.py fit_factory.py mod_dsp_gui.py run_type.py \
	trigger_filter.py cfd.py csra.py fit_gauss_creator.py \
	mod_dsp_layout.py system_toolbar.py widget_factory.py \
	chan_dsp_gui.py csrb.py fit_gauss_p1_creator.py mult_coincidence.py \
	tau.py worker.py chan_dsp_layout.py dsp_manager.py \
	fit_gauss_p2_creator.py pixie_utilities.py timing_control.py \
	xia_constants.py chan_dsp_manager.py dsp_toolbar.py fit_panel.py \
	plot.py trace_analyzer.py
