

lib_LTLIBRARIES=libddasformat.la

libddasformat_la_SOURCES = DDASHit.cpp DDASHitUnpacker.cpp
libddasformat_la_CPPFLAGS=-I@DAQINC@

include_HEADERS = DDASHit.h DDASHitUnpacker.h 

noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp \
		DDASHitTest.cpp \
		DDASUnpackerTest.cpp \
		DDASUnpackerTest250.cpp \
		DDASUnpackerTest250MSPS16Bit.cpp \
		DDASUnpackerTest500.cpp \
		Asserts.h DDASBitMasks.h

unittests_CXXFLAGS = @CPPUNIT_CFLAGS@
unittests_LDADD = @builddir@/libddasformat.la @CPPUNIT_LIBS@

TESTS = ./unittests
