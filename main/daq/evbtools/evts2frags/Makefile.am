noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp headerbuildertests.cpp Asserts.h

unittests_CPPFLAGS=@CPPUNIT_CFLAGS@
unittests_LDADD=@CPPUNIT_LDFLAGS@

check-TESTS:
	unittests
