bin_PROGRAMS = sbscaenupgrader

sbscaenupgrader_SOURCES = CAENComm_SBS.cpp

sbscaenupgrader_CPPFLAGS  = -I@top_srcdir@/caenfw-common -I@top_srcdir@/sbs/driver/include

sbscaenupgrader_LDADD = @top_builddir@/caenfw-common/libcaenfwloadCommon.la \
	@top_builddir@/sbs/driver/src/libbtp.la


EXTRA_DIST=sbscaenupgrader.xml
