evts2frag_COMMON_CFLAGS = -I@top_srcdir@/daq/format -I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/base/os


noinst_HEADERS=CFragmentMaker.h CEvents2Fragments.h

noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp headerbuildertests.cpp apptests.cpp \
	Asserts.h \
	CFragmentMaker.cpp CEvents2Fragments.cpp


unittests_CPPFLAGS=@CPPUNIT_CFLAGS@ $(evts2frag_COMMON_CFLAGS)
unittests_LDADD=@CPPUNIT_LDFLAGS@ @top_builddir@/base/os/libdaqshm.la

TESTS=unittests
