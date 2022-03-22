bin_PROGRAMS=glom

glom_SOURCES= glomMain.cpp CBufferedFragmentReader.cpp CBufferedFragmentReader.h \
	CEventAccumulatorSimple.cpp CEventAccumulatorSimple.h
nodist_glom_SOURCES=glom.c glom.h

BUILT_SOURCES = glom.c glom.h

glom_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format          \
	-I@top_srcdir@/base/thread		\
	-I@top_srcdir@/base/os @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@

glom_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la  \
	@top_builddir@/daq/format/libdataformat.la                     \
	@top_builddir@/base/os/libdaqshm.la @LIBEXCEPTION_LDFLAGS@

glom.c:  glom.h

glom.h: @srcdir@/glom.ggo
	@GENGETOPT@ --input=@srcdir@/glom.ggo \
		--output-dir=@builddir@ --file-name=glom

EXTRA_DIST=glom.ggo glom.xml


noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp Asserts.h bufffragreadertests.cpp evacctests.cpp \
	CBufferedFragmentReader.cpp \
	CEventAccumulator.cpp CEventAccumulatorSimple.cpp CEventAccumulatorSimple.h \
	simpleacctests.cpp integrationtests.cpp
unittests_CXXFLAGS=-I@top_srcdir@/daq/eventbuilder @CPPUNIT_CFLAGS@ \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/uri -I@top_srcdir@/daq/format \
	-I@top_srcdir@/daq/IO	\
	-I@top_srcdir@/base/os @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@

unittests_LDFLAGS=@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/daq/IO/libdaqio.la    \
	@top_builddir@/base/uri/liburl.la	\
	@top_builddir@/base/os/libdaqshm.la	\
	@CPPUNIT_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@

unittests_DEPENDENCIES=glom

TESTS=./unittests
