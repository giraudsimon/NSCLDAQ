bin_PROGRAMS = ringFragmentSource

ringFragmentSource_SOURCES=rfcmdline.c Main.cpp \
	CRingFragmentSource.h CRingFragmentSource.cpp
ringFragmentSource_CXXFLAGS=-I@top_srcdir@/daq/eventbuilder @LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/dataflow
ringFragmentSource_LDFLAGS=@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la 	\
	@LIBEXCEPTION_LDFLAGS@ -ldl


BUILT_SOURCES = rfcmdline.c rfcmdline.h



rfcmdline.c: rfcmdline.h


rfcmdline.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ \
		--input=@builddir@/options.ggo -F rfcmdline
