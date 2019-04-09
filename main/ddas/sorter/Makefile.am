bin_PROGRAMS=ddasSort

ddasSort_SOURCES=ddasSort.cpp ddasSortOptions.c ddasSortOptions.h \
	DDASSorter.cpp DDASSorter.h RawChannel.h RawChannel.cpp \
	ZeroCopyHit.h ZeroCopyHit.cpp BufferArena.h BufferArena.cpp \
	ReferenceCountedBuffer.h ReferenceCountedBuffer.cpp	\
	HitManager.h HitManager.cpp

ddasSort_CXXFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/dataflow  \
	@LIBTCLPLUS_CFLAGS@
ddasSort_LDFLAGS=@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la @LIBEXCEPTION_LDFLAGS@

BUILT_SOURCES=ddasSortOptions.c ddasSortOptions.h

ddasSortOptions.c: ddasSortOptions.h

ddasSortOptions.h:  ddasSortOptions.ggo
	$(GENGETOPT) <@srcdir@/ddasSortOptions.ggo \
		--file=ddasSortOptions --output-dir=@srcdir@
