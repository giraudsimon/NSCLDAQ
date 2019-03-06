bin_PROGRAMS    = ringFragmentSource
noinst_PROGRAMS = ringTestSource unittests srctests
lib_LTLIBRARIES= libTimeExtractorTest.la libS800TimeExtractor.la libGretinaTimeExtractor.la

BUILT_SOURCES = rfcmdline.c rfcmdline.h  \
	ringtestoptions.c ringtestoptions.h options.ggo


libTimeExtractorTest_la_SOURCES = testTimeExtractor.c	
libTimeExtractorTest_la_CPPFLAGS=-I@top_srcdir@/daq/format

libS800TimeExtractor_la_SOURCES = s800timestamp.c
libS800TimeExtractor_la_CPPFLAGS=-I@top_srcdir@/daq/format

libGretinaTimeExtractor_la_SOURCES = gretinaTimestamp.c
libGretinaTimeExtractor_la_CPPFLAGS=-I@top_srcdir@/daq/format


ringFragmentSource_SOURCES=	 CRingSource.cpp \
				 CRingItemToFragmentTransform.cpp \
				 Main.cpp GetOpt.cpp

nodist_ringFragmentSource_SOURCES=  rfcmdline.c rfcmdline.h

ringTestSource_SOURCES= ringTestSource.cpp TestSource.cpp
nodist_ringTestSource_SOURCES=ringtestoptions.c ringtestoptinos.h

ringTestSource_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format -I@top_srcdir@/base/os

noinst_HEADERS=CRingSource.h CRingItemToFragmentTransform.h s800.h TestSource.h



#
#  This is a bit funky.  We copy the options.ggo skeleton file and add to it
#  options needed for the ring buffer...then generate cmdline.{h,c}

options.ggo: @top_srcdir@/daq/eventbuilder/options.ggo
	cp @top_srcdir@/daq/eventbuilder/options.ggo  @builddir@
	echo option \"default-id\" S \"Source id to assign data without a predefined source id\" int optional >> @builddir@/options.ggo
	echo option \"ring\" r \"URL of ring from which fragments come\" string required >> @builddir@/options.ggo
	echo option \"timestampextractor\" t \"Shared library with timestamp extraction code\" string optional >> \
		@builddir@/options.ggo
	echo option \"expectbodyheaders\" e \"Body headers are expected on every ring item.\" flag off >> \
		@builddir@/options.ggo
	echo option \"oneshot\" o \"One shot after n end run items\" optional  int default=\"1\" >> @builddir@/options.ggo
	echo option \"timeout\" T \"Timeout waiting for end runs in oneshot mode\" int default=\"10\" optional  >>@builddir@/options.ggo
	echo option \"offset\"  O \"Signed time offset to add to the extracted timestamp\" int default=\"0\" optional >>@builddir@/options.ggo

rfcmdline.c: rfcmdline.h


rfcmdline.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ \
		--input=@builddir@/options.ggo -F rfcmdline

ringtestoptions.c: ringtestoptions.h


ringtestoptions.h: @srcdir@/ringtestoptions.ggo
	$(GENGETOPT) --input=@srcdir@/ringtestoptions.ggo \
		--output-dir=@builddir@ --file=ringtestoptions

ringTestSource_LDADD = \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBEXCEPTION_LDFLAGS@	\
	-ldl 

.FIRST: 
	rm -rf $(BUILT_SOURCES)

clean-local:
	-rm -rf $(BUILT_SOURCES) @srcdir@/$(BUILT_SOURCES)

ringFragmentSource_CPPFLAGS=-I@top_srcdir@/daq/eventbuilder -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format \
	@LIBTCLPLUS_CFLAGS@	 \
	 -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os

ringFragmentSource_LDADD=  		 \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/dataflow/libRingBuffer.la \
	@top_builddir@/base/os/libdaqshm.la	\
	$(THREAD_LDFLAGS)  -ldl

EXTRA_DIST=ringsource.xml ringtestoptions.ggo

unittests_SOURCES = TestRunner.cpp CRingSource.cpp CRingItemToFragmentTransform.cpp \
                    cringitemtofragtransformtests.cpp 

nodist_unittests_SOURCES = rfcmdline.c

unittests_LDADD = \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/dataflow/libRingBuffer.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@CPPUNIT_LDFLAGS@ \
	$(THREAD_LDFLAGS)  -ldl

unittests_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format \
	@LIBTCLPLUS_CFLAGS@	 \
	 -I@top_srcdir@/base/headers \
	@CPPUNIT_CFLAGS@ \
	-I@top_srcdir@/base/os


srctests_SOURCES=TestRunner.cpp CRingSource.cpp CRingItemToFragmentTransform.cpp \
                   cringsourcetests.cpp 

nodist_srctests_SOURCES=rfcmdline.c

srctests_LDADD = \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/dataflow/libRingBuffer.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@CPPUNIT_LDFLAGS@ \
	$(THREAD_LDFLAGS)  -ldl

srctests_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format \
	@LIBTCLPLUS_CFLAGS@	 \
	 -I@top_srcdir@/base/headers \
	@CPPUNIT_CFLAGS@ \
	-I@top_srcdir@/base/os

TESTS=	unittests srctests
