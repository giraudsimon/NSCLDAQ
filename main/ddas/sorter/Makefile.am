bin_PROGRAMS=ddasSort

ddasSort_SOURCES=ddasSort.cpp ddasSortOptions.c ddasSortOptions.h \
	DDASSorter.cpp DDASSorter.h RawChannel.h RawChannel.cpp \
	ZeroCopyHit.h ZeroCopyHit.cpp BufferArena.h BufferArena.cpp \
	ReferenceCountedBuffer.h ReferenceCountedBuffer.cpp	\
	HitManager.h HitManager.cpp

ddasSort_CPPFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/dataflow  \
	@LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@
ddasSort_LDFLAGS=@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la @LIBEXCEPTION_LDFLAGS@

BUILT_SOURCES=ddasSortOptions.c ddasSortOptions.h

ddasSortOptions.c: ddasSortOptions.h

ddasSortOptions.h:  ddasSortOptions.ggo
	$(GENGETOPT) <@srcdir@/ddasSortOptions.ggo \
		--file=ddasSortOptions --output-dir=@srcdir@

EXTRA_DIST=ddasSortOptions.ggo

#----------------------- unit tests

noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp Asserts.h hitmgrtests.cpp sortertests.cpp \
	refcountTests.cpp arenaTests.cpp rawchTests.cpp	zcopyhitTests.cpp \
	DDASSorter.cpp DDASSorter.h				\
	HitManager.cpp 	HitManager.h ZeroCopyHit.h ZeroCopyHit.cpp	\
	RawChannel.h RawChannel.cpp \
	ReferenceCountedBuffer.h ReferenceCountedBuffer.cpp \
	BufferArena.h BufferArena.cpp testcommon.cpp testcommon.h

unittests_CXXFLAGS=$(ddasSort_CPPFLAGS) @CPPUNIT_CFLAGS@ 
unittests_LDFLAGS=$(ddasSort_LDFLAGS) @CPPUNIT_LDFLAGS@

#  Fixed in 12.0-pre1
#TESTS=@builddir@/unittests
