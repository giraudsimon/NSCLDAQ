
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

bin_PROGRAMS=DDASReadout_extcl


DDASReadout_extcl_SOURCES=Skeleton.cpp CMyEventSegment.cpp CMyTrigger.cpp CMyBusy.cpp CMyScaler.cpp CMyEndCommand.cpp \
	Skeleton.h CMyEventSegment.h CMyTrigger.h CMyBusy.h CMyScaler.h CMyEndCommand.h \
	CSyncCommand.h CSyncCommand.cpp CBootCommand.cpp CBootCommand.h


DDASReadout_extcl_CPPFLAGS=-I@DDAS_INSTDIR@/include -I@top_srcdir@/daq/format \
	-I@top_srcdir@/sbs/readout -I@top_srcdir@/base/dataflow		\
	-I@top_srcdir@/sbs/nsclapi					\
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@         			\
	-fPIC -DFIRMWARE_FILE=\"$(SHAREDIR)/DDASFirmwareVersions.txt\"

DDASReadout_extcl_LDFLAGS= \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la   \
	@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	-L@DDAS_INSTDIR@/lib -lPixie16App -lPixie16Sys          \
	-lConfiguration -lSystemBooter @PLX_LDFLAGS@ 		\
	@LIBTCLPLUS_LDFLAGS@	@TCL_LDFLAGS@			\
	-Wl,"-rpath=@DDAS_INSTDIR@/lib"


#libddastimestampextractor_la_SOURCES=ddastimestamp.cpp
#libddastimestampextractor_la_CPPFLAGS=$(Readout_extcl_CPPFLAGS) $(CXXFLAGS)


#
#   *  Install the firmware versions file in @prefix@/share/readout
#   *  Install the crate_1 dir in @prefix@/share/readout/crate_1
#

SHAREDIR=@prefix@/share/ddasreadout-extcl

install-exec-hook:
	$(mkinstalldirs) $(SHAREDIR)
	$(mkinstalldirs) $(SHAREDIR)/crate_1
	$(INSTALL_DATA) DDASFirmwareVersions_ExtClock.txt $(SHAREDIR)


EXTRA_DIST= DDASFirmwareVersions_ExtClock.txt.in 
