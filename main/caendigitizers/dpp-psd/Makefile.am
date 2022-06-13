lib_LTLIBRARIES=libCAENDPP_PSD.la

libCAENDPP_PSD_la_CXXFLAGS=$(CAENCCFLAGS) -I@top_srcdir@/sbs/readout -I@top_srcdir@/base/pugi \
	-I@top_srcdir@/base/iniparser

libCAENDPP_PSD_la_LDFLAGS=$(CAENLDFLAGS) @top_builddir@/sbs/readout/libSBSProductionReadout.la  \
	@top_builddir@/base/pugi/libpugixml.la

libCAENDPP_PSD_la_SOURCES=CCompoundTrigger.cpp  CDPpPsdEventSegment.cpp \
	COneOnlyEventSegment.cpp  CPsdCompoundEventSegment.cpp  \
	CPsdTrigger.cpp  PSDParameters.cpp CPSDMaker.cpp


include_HEADERS=CCompoundTrigger.h  CDPpPsdEventSegment.h  \
	COneOnlyEventSegment.h  CPsdCompoundEventSegment.h \
	CPsdTrigger.h  PSDParameters.h CPSDMaker.h

install-exec-hook:
	$(mkinstalldirs) @prefix@/skeletons/dpp-psd
	$(INSTALL_DATA) @srcdir@/skel/*.cpp @prefix@/skeletons/dpp-psd
	$(INSTALL_DATA) @srcdir@/skel/*.h   @prefix@/skeletons/dpp-psd
	$(INSTALL_DATA) @builddir@/skel/Makefile @prefix@/skeletons/dpp-psd

EXTRA_DIST=@srcdir@/skel/*.cpp @srcdir@/skel/*.h @srcdir@/skel/Makefile.in
