noinst_PROGRAMS =  unittests

unittests_SOURCES=AmplitudeProcessorTest.cpp \
	AmplitudeProcessorTest.h \
	BaseLineProcessorTest.cpp \
	BaseLineProcessorTest.h \
	BasicIteratorTest.cpp \
	BasicIteratorTest.h \
	BasicRangeIteratorTest.cpp \
	BasicRangeIteratorTest.h \
	CFDTest.cpp \
	CFDTest.h \
	PeakFindProcessorTest.cpp \
	PeakFindProcessorTest.h \
	RiseTimeProcessorTest.cpp \
	RiseTimeProcessorTest.h \
	SumIteratorTest.cpp \
	SumIteratorTest.h \
	TestMain.cpp \
	ThresholdTest.cpp \
	ThresholdTest.h \
	TraceSTest.cpp \
	TraceSTest.h \
	TrapFilterTest.cpp \
	TrapFilterTest.h

unittests_CPPFLAGS= -I@srcdir@/../src  @ROOT_CFLAGS@
unittests_LDFLAGS = @builddir@/../src/libtraiter.la @ROOT_LDFLAGS@ -lcppunit

TESTS=unittests
