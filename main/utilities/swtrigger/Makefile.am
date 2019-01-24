
lib_LTLIBRARIES = libSwTrigger.la

libSwTrigger_la_SOURCES = ProcessingElement.cpp ZMQPushDataSource.cpp \
	ZMQContext.cpp swFilterRingDataSource.cpp

libSwTrigger_la_CXXFLAGS=-I@top_srcdir@/base/thread -I@top_srcdir@/daq/format \
	-I@top_srcdir@/utilities/IO
libSwTrigger_la_LDFLAGS=@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/utilities/IO/libdaqio.la	\
	@top_builddir@/base/thread/libdaqthreads.la @ZMQ_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@

swfilterdir=@includedir@/swfilter

swfilter_HEADERS=MessageTypes.h ZMQPushDataSource.h  \
	ZMQContext.h zhelpers.hpp \
	ProcessingElement.h zmqServices.h swFilterRingDataSource.h

noinst_HEADERS=zmqServices.h 



noinst_PROGRAMS = unittests

unittests_SOURCES=TestRunner.cpp swfiltersrctests.cpp Asserts.h \
		swFilterRingDataSource.cpp
unittests_CXXFLAGS=@CPPUNIT_CFLAGS@ $(libSwTrigger_la_CXXFLAGS) @LIBTCLPLUS_CFLAGS@
unittests_LDFLAGS= @CPPUNIT_LDFLAGS@ $(libSwTrigger_la_LDFLAGS)

TESTS=unittests

