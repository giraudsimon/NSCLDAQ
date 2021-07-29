bin_PROGRAMS = ringFragmentSource

ringFragmentSource_SOURCES=rfcmdline.c Main.cpp \
	CRingFragmentSource.h CRingFragmentSource.cpp
ringFragmentSource_CXXFLAGS=-I@top_srcdir@/daq/eventbuilder @LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/dataflow -I@top_srcdir@/daq/format
ringFragmentSource_LDFLAGS=@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la 	\
	@LIBEXCEPTION_LDFLAGS@ -ldl


BUILT_SOURCES = rfcmdline.c rfcmdline.h

EXTRA_DIST=  options.ggo manpages.xml


rfcmdline.c: rfcmdline.h


rfcmdline.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ \
		--input=@srcdir@/options.ggo -F rfcmdline


noinst_PROGRAMS= unittests

unittests_SOURCES=TestRunner.cpp fragsrctests.cpp Asserts.h \
	CRingFragmentSource.cpp 
unittests_CXXFLAGS= $(ringFragmentSource_CXXFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS= @CPPUNIT_LDFLAGS@ @top_builddir@/daq/format/libdataformat.la \
	-ldl

TESTS=./unittests
