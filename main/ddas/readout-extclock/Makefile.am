
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
	@PIXIE_CPPFLAGS@					\
	-I@top_srcdir@/sbs/readout -I@top_srcdir@/base/dataflow		\
	-I@top_srcdir@/sbs/nsclapi					\
	-I@top_srcdir@/base/os						\
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@         			\
	@PIXIE_CPPFLAGS@			\
	-fPIC -DFIRMWARE_FILE=\"@DDAS_INSTDIR@/share/readout/DDASFirmwareVersions_ExtClock.txt\"

DDASReadout_extcl_LDFLAGS= \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la   \
	@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/os/libdaqshm.la			\
	-L@DDAS_INSTDIR@/lib @PIXIE_LDFLAGS@          \
	-lConfiguration -lSystemBooter @PLX_LDFLAGS@ 		\
	@LIBTCLPLUS_LDFLAGS@	@TCL_LDFLAGS@			\
	-Wl,"-rpath=@DDAS_INSTDIR@/lib"


