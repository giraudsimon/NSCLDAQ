lib_LTLIBRARIES=libLogbook.la

libLogbook_la_SOURCES=LogBook.cpp LogBookPerson.cpp LogBookShift.cpp
libLogbook_la_CXXFLAGS=@SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqliteplus
libLogbook_la_LDFLAGS=@top_builddir@/base/sqliteplus/libSqlite3pp.la @SQLITE3_LIBS@

include_HEADERS=LogBook.h LogBookPerson.h LogBookShift.h


noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp excepttests.cpp \
	bookstatictests.cpp logbooktests.cpp persontests.cpp personapitest.cpp \
	shifttests.cpp shiftapitest.cpp

unittests_CXXFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base/testutils $(libLogbook_la_CXXFLAGS)
unittests_LDFLAGS=libLogbook.la $(libLogbook_la_LDFLAGS) @CPPUNIT_LDFLAGS@
unittests_DEPENDENCIES=libLogbook.la

TESTS=unittests
