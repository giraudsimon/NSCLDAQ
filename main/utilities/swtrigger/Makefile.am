lib_LTLIBRARIES=libSwTrigger.la

libSwTrigger_la_SOURCES=CReceiver.cpp CTestTransport.cpp CTestTransport.h zmqhelpers.hpp \
	CZMQTransport.cpp CZMQClientTransport.cpp CZMQServerTransport.cpp \
	CSender.cpp CRingItemFileTransport.cpp CRingBufferTransport.cpp \
	CDispatcher.cpp CClientRegistry.cpp CZMQRouterTransport.cpp \
	CZMQDealerTransport.cpp CDataSourceElement.cpp CParallelWorker.cpp \
	CDataSinkElement.cpp CThreadedProcessingElement.cpp		\
	CRingItemTransportFactory.cpp CRingItemDispatcher.cpp		\
	CZMQRingItemDispatcher.cpp CRingItemZMQSourceElement.cpp        \
	CZMQRingItemSourceThread.cpp CZMQRingItemWorker.cpp	\
	CZMQRingItemThreadedWorker.cpp CZMQCommunicatorFactory.cpp \
	CCommunicatorFactoryMaker.cpp CRingItemMarkingWorker.cpp

libSwTrigger_la_LDFLAGS=@top_builddir@/base/os/libdaqshm.la           \
	@top_builddir@/daq/format/libdataformat.la 		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/uri/liburl.la			\
	@top_builddir@/base/dataflow/libDataFlow.la		\
	@LIBEXCEPTION_LDFLAGS@ @ZMQ_LDFLAGS@

libSwTrigger_la_CXXFLAGS=-I@top_srcdir@/utilities/IO -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os -I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/factories	\
	-I@top_srcdir@/base/thread -I@top_srcdir@/base/uri @LIBTCLPLUS_CFLAGS@

include_HEADERS = CTransport.h CReceiver.h CZMQTransport.h CZMQClientTransport.h \
	CZMQServerTransport.h CSender.h CRingItemTransport.h CRingItemFileTransport.h \
	CRingBufferTransport.h CDispatcher.h CClientRegistry.h CZMQRouterTransport.h \
	CZMQDealerTransport.h CFanoutClientTransport.h CProcessingElement.h \
	CDataSourceElement.h CParallelWorker.h CDataSinkElement.h  \
	CThreadedProcessingElement.h CRingItemTransportFactory.h	\
	CRingItemDispatcher.h CZMQRingItemDispatcher.h CRingItemZMQSourceElement.h \
	CZMQRingItemSourceThread.h CZMQRingItemWorker.h CProcessor.h \
	CZMQRingItemThreadedWorker.h CCommunicatorFactory.h CZMQCommunicatorFactory.h \
	CCommunicatorFactoryMaker.h CRingItemMarkingWorker.h

BUILT_SOURCES=swtriggerflags.c swtriggerflags.h

swtriggerflags.c: swtriggerflags.h

swtriggerflags.h: swtriggerflags.ggo
	$(GENGETOPT) <@srcdir@/swtriggerflags.ggo -Fswtriggerflags

EXTRA_DIST=swtriggerflags.ggo

####################################################################################
# unit testing:

noinst_PROGRAMS=unittests
unittests_SOURCES=TestRunner.cpp Asserts.h testxporttests.cpp zmqxporttests.cpp \
	zmqsvrxporttests.cpp zmqclitests.cpp zmqdealertests.cpp zmqrtrtests.cpp \
	zmqsendertests.cpp zmqreceivertests.cpp zmqdispattests.cpp	\
	ritemfxporttests.cpp rbufferxporttests.cpp ritemtransportfactoryTests.cpp	\
	dsrceltests.cpp	pworkertests.cpp sinktests.cpp tprocesstests.cpp \
	zmqworkertests.cpp	\
	$(libSwTrigger_la_SOURCES)
unittests_CXXFLAGS=$(libSwTrigger_la_CXXFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS= @top_builddir@/utilities/IO/libdaqio.la \
	$(libSwTrigger_la_LDFLAGS) @CPPUNIT_LDFLAGS@


TESTS=./unittests
