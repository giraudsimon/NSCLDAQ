
lib_LTLIBRARIES = libSwTrigger.la

libSwTrigger_la_SOURCES = ProcessingElement.cpp ZMQPushDataSource.cpp \
	ZMQContext.cpp swFilterRingDataSource.cpp SingleRingItemConverter.cpp \
	swFilterRingBlockDataSource.cpp RingItemBlockConverter.cpp \
	CRingItemPipelineProcessor.cpp CRingItemDataSource.cpp \
	CRingItemDataSink.cpp ZMQFanInRingSource.cpp ZMQFanInRingSink.cpp \
	ZMQFanoutBalancedRingSink.cpp ZMQFanoutBalancedRingSource.cpp

libSwTrigger_la_CXXFLAGS= -I@top_srcdir@/base/os \
	-I@top_srcdir@/base/thread -I@top_srcdir@/daq/format \
	-I@top_srcdir@/utilities/IO
libSwTrigger_la_LDFLAGS= @top_builddir@/base/os/libdaqshm.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/utilities/IO/libdaqio.la	\
	@top_builddir@/base/thread/libdaqthreads.la @ZMQ_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@

swfilterdir=@includedir@/swfilter

swfilter_HEADERS=MessageTypes.h ZMQPushDataSource.h  \
	ZMQContext.h \
	ProcessingElement.h zmqServices.h swFilterRingDataSource.h \
	SingleRingItemConverter.h swFilterRingBlockDataSource.h RingItemBlockConverter.h \
	CRingItemDataSource.h CRingItemDataSink.h CRingItemProcessor.h \
	CRingItemPipelineProcessor.h ZMQFanInRingSource.h ZMQFanInRingSink.h \
	ZMQFanoutBalancedRingSink.h ZMQFanoutBalancedRingSource.h

noinst_HEADERS=zmqServices.h zhelpers.hpp



noinst_PROGRAMS = unittests

unittests_SOURCES=TestRunner.cpp swfiltersrctests.cpp singlritemconvtests.cpp \
		ritemblocksrctests.cpp rblockcvttest.cpp  \
		zmqpushsrctest.cpp zmqpushblocktest.cpp \
		Asserts.h fakedatasource.h fakedatasource.cpp \
		$(libSwTrigger_la_SOURCES)

unittests_CXXFLAGS=@CPPUNIT_CFLAGS@ $(libSwTrigger_la_CXXFLAGS) @LIBTCLPLUS_CFLAGS@
unittests_LDFLAGS= @CPPUNIT_LDFLAGS@ $(libSwTrigger_la_LDFLAGS)

TESTS=unittests

