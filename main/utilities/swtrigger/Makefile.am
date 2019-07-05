
if MPI

## Rules for compiling MPI sources -- sources must have name of the form e.g.
#  file_mpi.cpp

AM_V_MPICC = $(am__v_MPICC_$(V))
am__v_MPICC_ = $(am__v_MPICC_$(AM_DEFAULT_VERBOSITY))
am__v_MPICC_0 = @echo "  MPICC " $@;
AM_V_MPICXX = $(am__v_MPICXX_$(V))
am__v_MPICXX_ = $(am__v_MPICXX_$(AM_DEFAULT_VERBOSITY))
am__v_MPICXX_0 = @echo "  MPICXX" $@;
MPICOMPILE=$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
        $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
MPICXXLD=$(MPICXX)
MPICXXCOMPILE=$(MPICXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
        $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
LTMPICOMPILE=$(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(MPICOMPILE)
LTMPICXXCOMPILE=$(LIBTOOL) $(AM_V_lt) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(MPICXXCOMPILE)

SUFFIXES = _mpi.cpp  _mpi.o _mpi.lo _mpi.obj


_mpi.cpp_mpi.o:
@am__fastdepCXX_TRUE@	$(AM_V_MPICXX)$(MPICXXCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*_mpi.Tpo -c -o $@ $<
@am__fastdepCXX_TRUE@	$(AM_V_at)$(am__mv) $(DEPDIR)/$*_mpi.Tpo $(DEPDIR)/$*_mpi.Po
@am__fastdepCXX_FALSE@	$(AM_V_MPICXX) @AM_BACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	DEPDIR=$(DEPDIR) $(CXXDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCXX_FALSE@	$(MPICXXCOMPILE) -c -o $@ $<

_mpi.cpp_mpi.lo:
@am__fastdepCXX_TRUE@	$(AM_V_MPICXX)$(LTMPICXXCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*_mpi.Tpo -c -o $@ $<
@am__fastdepCXX_TRUE@	$(AM_V_at)$(am__mv) $(DEPDIR)/$*_mpi.Tpo $(DEPDIR)/$*_mpi.Plo
@am__fastdepCXX_FALSE@	$(AM_V_MPICXX) @AM_BACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	source='$<' object='$@' libtool=yes @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	DEPDIR=$(DEPDIR) $(CXXDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCXX_FALSE@	$(LTMPICXXCOMPILE) -c -o $@ $<

_mpi.cpp_mpi.obj:
@am__fastdepCXX_TRUE@	$(AM_V_MPICXX)$(MPICXXCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*_mpi.Tpo -c -o $@ `$(CYGPATH_W) '$<'`
@am__fastdepCXX_TRUE@	$(AM_V_at)$(am__mv) $(DEPDIR)/$*_mpi.Tpo $(DEPDIR)/$*_mpi.Po
@am__fastdepCXX_FALSE@	$(AM_V_MPICXX) @AM_BACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	DEPDIR=$(DEPDIR) $(CXXDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCXX_FALSE@	$(MPICXXCOMPILE) -c -o $@ `$(CYGPATH_W) '$<'`

#  Define the transport sources and headers.

MPI_TRANSPORT_SRCS=CMPITransport_mpi.cpp
MPI_TRANSPORT_HDRS=CMPITransport_mpi.h

# For each of the SRCS above <sigh> we need the following to make libtool happy
# e.g. for test_mpi.cpp:

libSwTrigger_la-CMPITransport_mpi.o : CMPITransport_mpi.o
	cp CMPITransport_mpi.o libSwTrigger_la-CMPITransport_mpi.o
libSwTrigger_la-CMPITransport_mpi.lo :  CMPITransport_mpi.lo
	cp CMPITransport_mpi.lo libSwTrigger_la-CMPITransport_mpi.lo
unittests-CMPITransport_mpi.o : CMPITransport_mpi.o
	cp CMPITransport_mpi.o unittests-CMPITransport_mpi.o
unittests-CMPITransport_mpi.lo : CMPITransport_mpi.lo
	cp CMPITransport_mpi.lo unittests-CMPITransport_mpi.lo



#   For the library:

#libSwTrigger_la-test_mpi.o : test_mpi.o
#	cp test_mpi.o libSwTrigger_la-test_mpi.o
#libSwTrigger_la-test_mpi.lo : test_mpi.lo
#	cp test_mpi.lo libSwTrigger_la-test_mpi.lo

#  For unittests

#unittests-test_mpi.o: test_mpi.o
#	cp test_mpi.o unittests-test_mpi.o
#unittests-test_mpi.lo: test_mpi.lo
#	cp test_mpi.lo unittests-test_mpi.lo

# Yes this is a maintenance pain in the ass.



# Below will cause warning from auto make but will work at least for now.
# It's needed to get the unittests to use MPICXX as the linker.

CXXLD=$(MPICXX)

endif

lib_LTLIBRARIES=libSwTrigger.la libTestClassifier.la

bin_PROGRAMS=SoftwareTrigger EventFilter


libSwTrigger_la_SOURCES=CReceiver.cpp CTestTransport.cpp CTestTransport.h zhelpers.hpp \
	CZMQTransport.cpp CZMQClientTransport.cpp CZMQServerTransport.cpp \
	CSender.cpp CRingItemFileTransport.cpp CRingBufferTransport.cpp \
	CDispatcher.cpp CClientRegistry.cpp CZMQRouterTransport.cpp \
	CZMQDealerTransport.cpp CDataSourceElement.cpp CParallelWorker.cpp \
	CDataSinkElement.cpp CThreadedProcessingElement.cpp		\
	CRingItemTransportFactory.cpp CRingItemDispatcher.cpp		\
	CZMQRingItemDispatcher.cpp CRingItemZMQSourceElement.cpp        \
	CZMQRingItemSourceThread.cpp CZMQRingItemWorker.cpp	\
	CZMQRingItemThreadedWorker.cpp CZMQCommunicatorFactory.cpp \
	CCommunicatorFactoryMaker.cpp CRingItemMarkingWorker.cpp \
	CNullTransport.cpp CGather.cpp $(MPI_TRANSPORT_SRCS)

libSwTrigger_la_LDFLAGS=@top_builddir@/base/os/libdaqshm.la           \
	@top_builddir@/daq/format/libdataformat.la 		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/uri/liburl.la			\
	@top_builddir@/base/dataflow/libDataFlow.la		\
	@LIBEXCEPTION_LDFLAGS@ @ZMQ_LDFLAGS@ @THREADLD_FLAGS@

libSwTrigger_la_CXXFLAGS=-I@top_srcdir@/utilities/IO -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os -I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/factories	\
	-I@top_srcdir@/base/thread -I@top_srcdir@/base/uri -I@top_srcdir@/daq/eventbuilder \
	@LIBTCLPLUS_CFLAGS@ @THREADCXX_FLAGS@

include_HEADERS = CTransport.h CReceiver.h CZMQTransport.h CZMQClientTransport.h \
	CZMQServerTransport.h CSender.h CRingItemTransport.h CRingItemFileTransport.h \
	CRingBufferTransport.h CDispatcher.h CClientRegistry.h CZMQRouterTransport.h \
	CZMQDealerTransport.h CFanoutClientTransport.h CProcessingElement.h \
	CDataSourceElement.h CParallelWorker.h CDataSinkElement.h  \
	CThreadedProcessingElement.h CRingItemTransportFactory.h	\
	CRingItemDispatcher.h CZMQRingItemDispatcher.h CRingItemZMQSourceElement.h \
	CZMQRingItemSourceThread.h CZMQRingItemWorker.h CProcessor.h \
	CZMQRingItemThreadedWorker.h CCommunicatorFactory.h CZMQCommunicatorFactory.h \
	CCommunicatorFactoryMaker.h CRingItemMarkingWorker.h CNullTransport.h \
	CFanoutTransport.h CGather.h $(MPI_TRANSPORT_HDRS)


SoftwareTrigger_SOURCES=swtriggerflags.c swtriggerflags.h SoftwareTrigger.cpp \
	CClassifierApp.h \
	CZMQThreadedClassifierApp.cpp CZMQThreadedClassifierApp.h \
	CRingItemSorter.h CRingItemSorter.cpp CRingBlockDataSink.h \
	CRingBlockDataSink.cpp

SoftwareTrigger_DEPENDENCIES=libSwTrigger.la



SoftwareTrigger_CXXFLAGS=$(libSwTrigger_la_CXXFLAGS)
SoftwareTrigger_LDFLAGS=libSwTrigger.la $(libSwTrigger_la_LDFLAGS) -ldl

EventFilter_SOURCES=eventfilterflags.c eventfilterflags.h EventFilterMain.cpp \
	CEventFilterApp.cpp CEventFilterApp.h

EventFilter_Dependencies=libSwTrigger.la

EventFilter_CXXFLAGS=$(libSwTrigger_la_CXXFLAGS)
EventFilter_LDFLAGS= libSwTrigger.la $(libSwTrigger_la_LDFLAGS)

libTestClassifier_la_SOURCES=TestClassifier.cpp
libTestClassifier_la_CXXFLAGS=$(libSwTrigger_la_CXXFLAGS)
libTestClassifier_la_LDFLAGS=libSwTrigger.la $(libSwTrigger_la_LDFLAGS)

BUILT_SOURCES=swtriggerflags.c swtriggerflags.h eventfilterflags.c eventfilterflags.h

swtriggerflags.c: swtriggerflags.h

swtriggerflags.h: swtriggerflags.ggo
	$(GENGETOPT) <@srcdir@/swtriggerflags.ggo -Fswtriggerflags

eventfilterflags.c: eventfilterflags.h

eventfilterflags.h: eventfilterflags.ggo
	$(GENGETOPT) <@srcdir@/eventfilterflags.ggo -Feventfilterflags


EXTRA_DIST=swtriggerflags.ggo eventfilterflags.ggo ConceptsAndPrograms.xml \
	CommFramework.xml

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

unittests_DEPENDENCIES=libSwTrigger.la

unittests_CXXFLAGS=$(libSwTrigger_la_CXXFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS= @top_builddir@/utilities/IO/libdaqio.la \
	$(libSwTrigger_la_LDFLAGS) @CPPUNIT_LDFLAGS@


UNITTESTS: unittests
	./unittests
