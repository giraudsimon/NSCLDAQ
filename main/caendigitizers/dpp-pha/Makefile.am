lib_LTLIBRARIES=libCAENDPP_PHA.la

libCAENDPP_PHA_la_CXXFLAGS=$(CAENCCFLAGS) -I@top_srcdir@/sbs/readout \
	-I@top_srcdir@/base/pugi \
	-I@top_srcdir@/base/iniparser
libCAENDPP_PHA_la_LDFLAGS=$(CAENLDFLAGS) \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la  \
	@top_builddir@/base/pugi/libpugixml.la  \
	@top_builddir@/base/iniparser/libiniparser.la

libCAENDPP_PHA_la_SOURCES=CAENPhaChannelParameters.cpp  CAENPhaParameters.cpp \
	CompassEventSegment.cpp  CompassProject.cpp  config.cpp     PHAEventSegment.cpp  \
	PHATrigger.cpp CAENPha.cpp  CAENPHAScalers.cpp     CompassMultiModuleEventSegment.cpp  \
	CompassTrigger.cpp  DPPConfig.cpp  PHAMultiModuleSegment.cpp 



include_HEADERS=CAENPhaChannelParameters.h  CAENPhaParameters.h  CompassEventSegment.h    \
	CompassProject.h  config.h     PHAEventSegment.h        PHATrigger.h \
	CAENPha.h                   CAENPHAScalers.h     CompassMultiModuleEventSegment.h \
	CompassTrigger.h  DPPConfig.h  PHAMultiModuleSegment.h CAENPhaBuffer.h

install-exec-hook:
	$(mkinstalldirs) @prefix@/skeletons/dpp-pha
	$(INSTALL_DATA) @srcdir@/skel/*.cpp @prefix@/skeletons/dpp-pha
	$(INSTALL_DATA) @srcdir@/skel/*.h   @prefix@/skeletons/dpp-pha
	$(INSTALL_DATA) @builddir@/skel/Makefile @prefix@/skeletons/dpp-pha


EXTRA_DIST=@srcdir@/skel/*.cpp @srcdir@/skel/*.h @srcdir@/skel/Makefile.in
