#
# This requires that DDAS_INSTDIR be defined
# This gets defined with --enable-ddas=path-to-ddas-installation
#
# We also require PLX_LDFLAGS, PLX_CCFLAGS  be defined.  If --enable-ddas can't
# figure it out by knowing where fthe PLX_SDK_DIR is, the user
# must help out with:
#
#  --with-plx-sdk=path-to-plx-directory-tree.
#

bin_PROGRAMS=DDASReadout
lib_LTLIBRARIES=libddastimestampextractor.la
SHAREDIR=@prefix@/share/ddasreadout




DDASReadout_SOURCES=Skeleton.cpp CMyEventSegment.cpp CMyTrigger.cpp CMyBusy.cpp CMyScaler.cpp CMyEndCommand.cpp \
	Skeleton.h CMyEventSegment.h CMyTrigger.h CMyBusy.h CMyScaler.h \
	CMyEndCommand.h CSyncCommand.cpp CSyncCommand.h CBootCommand.cpp \
	CBootCommand.h RawChannel.cpp RawChannel.h 			\
	ReferenceCountedBuffer.cpp ReferenceCountedBuffer.h 		\
	BufferArena.cpp BufferArena.h ZeroCopyHit.cpp ZeroCopyHit.h	\
	ModuleReader.cpp ModuleReader.h CHitManager.cpp CHitManager.h \
	CDDASStatisticsCommand.h CDDASStatisticsCommand.cpp


DDASReadout_CPPFLAGS=-I@DDAS_INSTDIR@/include -I@top_srcdir@/daq/format \
	@PIXIE_CPPFLAGS@ \
	-I@top_srcdir@/sbs/readout -I@top_srcdir@/base/dataflow		\
	-I@top_srcdir@/sbs/nsclapi					\
	-I@top_srcdir@/base/os						\
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@         			\
	@PIXIE_CPPFLAGS@						\
	-fPIC -DFIRMWARE_FILE=\"@DDAS_INSTDIR@/share/readout/DDASFirmwareVersions.txt\" \
	-DSORTING

DDASReadout_LDFLAGS= \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la   \
	@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/os/libdaqshm.la			\
	-L@DDAS_INSTDIR@/lib @PIXIE_LDFLAGS@			\
	-lConfiguration -lSystemBooter @PLX_LDFLAGS@ 		\
	@LIBTCLPLUS_LDFLAGS@	@TCL_LDFLAGS@			\
	-Wl,"-rpath=@DDAS_INSTDIR@/lib"


libddastimestampextractor_la_SOURCES=ddastimestamp.cpp
libddastimestampextractor_la_CPPFLAGS=$(DDASReadout_CPPFLAGS) $(CXXFLAGS)


#
#   *  Install the firmware versions file in @prefix@/share/readout
#   *  Install the crate_1 dir in @prefix@/share/readout/crate_1
#


install-exec-hook:
	$(mkinstalldirs) $(SHAREDIR)
	$(mkinstalldirs) $(SHAREDIR)/crate_1
	$(INSTALL_DATA) @srcdir@/crate_1/*       $(SHAREDIR)/crate_1
	$(INSTALL_DATA) @srcdir@/envset          $(SHAREDIR)

EXTRA_DIST=crate_1 envset README
