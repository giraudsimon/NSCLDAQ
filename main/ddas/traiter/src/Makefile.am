lib_LTLIBRARIES = libtraiter.la

libtraiter_la_SOURCES = Solver.cpp Exceptions.cpp dicttraiter.cpp

libtraiter_la_CPPFLAGS = @ROOT_CFLAGS@
libtraiter_la_LDFLAGS = @ROOT_LDFLAGS@

LIBHEADERS = AlgoIterator.hpp AmplitudeProcessor.hpp BaseLineProcessor.hpp \
	CFD.hpp PeakFindProcessor.hpp RiseTimeProcessor.hpp \
	SumIterator.hpp Threshold.hpp Trace.hpp \
	TrapFilter.hpp TrIterator.hpp

include_HEADERS = Exceptions.h LinkDef.h Solver.h TraceAlgorithms.h \
	TraceDefs.h \
	$(LIBHEADERS) 
BUILT_SOURCES = dicttraiter.cpp dictraiter.h

dicttraiter.cpp: dictraiter.h

dictraiter.h: LinkDef.h
	LD_LIBRARY_PATH=@ROOTSYS@/lib @ROOTSYS@/bin/rootcint \
		-f dicttraiter.cpp -c -p -I@srcdir@ $(LIBHEADERS) @srcdir@/LinkDef.h

# Used to have @ROOT_CFLAGS@


