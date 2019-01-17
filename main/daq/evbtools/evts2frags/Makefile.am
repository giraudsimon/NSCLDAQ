evts2frag_COMMON_CFLAGS = -I@top_srcdir@/daq/format -I@top_srcdir@/daq/eventbuilder


noinst_HEADERS=CFragmentMaker.h CEvents2Fragments.h

noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp headerbuildertests.cpp Asserts.h \
	CFragmentMaker.cpp CEvents2Fragments.cpp


unittests_CPPFLAGS=@CPPUNIT_CFLAGS@ $(evts2frag_COMMON_CFLAGS)
unittests_LDADD=@CPPUNIT_LDFLAGS@

TESTS=unittests
