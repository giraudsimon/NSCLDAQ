bin_PROGRAMS=nscope.bin 
noinst_PROGRAMS = unittests
lib_LTLIBRARIES = libNscope.la

libNscope_la_SOURCES=AnalogSignal.cpp ExpertMod.cpp MaxEvent.cpp Baseline.cpp Histogram.cpp Csra.cpp ModCsrb.cpp \
	TrigConfig.cpp Detector.cpp displayEdit.cpp EnergyFilter.cpp PulseShape.cpp ScopedT.cpp Table.cpp \
	Tau.cpp TriggerFilter.cpp CFD.cpp Popup.cpp lock.cpp MultCoincDialogue.cpp MultiplicityState.cpp \
	 TimingParamDialogue.cpp Main.cpp MediaLocator.cpp \
	AnalogSignal.h Detector.h ExpertMod.h MaxEvent.h Table.h Baseline.h displayEdit.h Histogram.h \
	PulseShape.h Tau.h Csra.h ModCsrb.h TrigConfig.h EnergyFilter.h ScopedT.h TriggerFilter.h CFD.h \
	Popup.h lock.h MultCoincDialogue.h MultiplicityState.h TimingParamDialogue.h Main.h MediaLocator.h QDCLen.h QDCLen.cpp

libNscope_la_CPPFLAGS=-g -Wall @PLX_CPPFLAGS@ \
	@PIXIE_CPPFLAGS@  @ROOT_CFLAGS@ -DFIRMWARE_FILE=\"@prefix@/share/ddasreadout/DDASFirmwareVersions.txt\" \
        -DPREFIX='"@prefix@"' -I@top_srcdir@ -I@top_srcdir@/ddas/configuration -I@top_srcdir@/ddas/booter
libNscope_la_LIBADD=@PIXIE_LDFLAGS@ \
        @ROOT_LDFLAGS@ @PLX_LDFLAGS@ -ldl @top_builddir@/ddas/configuration/libConfiguration.la \
        @top_builddir@/ddas/booter/libSystemBooter.la

nscope_bin_SOURCES = nscope.cpp
nscope_bin_CPPFLAGS =$(libNscope_la_CPPFLAGS)
nscope_bin_LDADD = @builddir@/libNscope.la \
	@PIXIE_LDFLAGS@ \
	@ROOT_LDFLAGS@ \
	@PLX_LDFLAGS@ -ldl

EXTRA_DIST=README_nscope.txt @srcdir@/figures/* @srcdir@/resources/*

RESOURCES = @srcdir@/resources/CSRAON.png @srcdir@/resources/CSRAOFF.png @srcdir@/resources/CSRAunknown.png \
@srcdir@/resources/TimingParamDialogueDiagram.png

install-data-local:
	$(mkinstalldirs) @prefix@/share/html
	$(mkinstalldirs) @prefix@/share/html/figures
	$(mkinstalldirs) @prefix@/share/nscope
	for f in $(RESOURCES) ; do $(INSTALL_DATA) $$f @prefix@/share/html/figures/. ; done
	for f in $(RESOURCES) ; do $(INSTALL_DATA) $$f @prefix@/share/nscope/. ; done

install-exec-local:
	$(mkinstalldirs) @prefix@/bin
	$(INSTALL_PROGRAM) nscope @prefix@/bin

unittests_SOURCES = TestRunner.cpp \
										MediaLocatorTests.cpp

unittests_CPPFLAGS = @CPPUNIT_CFLAGS@ -I@srcdir@ -I@top_srcdir@
unittests_LDADD    = $(nscope_bin_LDADD) @CPPUNIT_LIBS@

TESTS = unittests
