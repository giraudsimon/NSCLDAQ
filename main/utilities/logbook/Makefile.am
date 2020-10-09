lib_LTLIBRARIES=libLogbook.la libPyLogbook.la

libLogbook_la_SOURCES=LogBook.cpp LogBookPerson.cpp LogBookShift.cpp \
	LogBookRun.cpp LogBookNote.cpp
libLogbook_la_CXXFLAGS=@SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqliteplus
libLogbook_la_LDFLAGS=@top_builddir@/base/sqliteplus/libSqlite3pp.la \
	@SQLITE3_LIBS@


libPyLogbook_la_DEPENDENCIES=libLogbook.la
libPyLogbook_la_CXXFLAGS=$(libLogbook_la_CXXFLAGS) @PYTHON3_CFLAGS@
libPyLogbook_la_LDFLAGS= libLogbook.la $(libLogbook_la_LDFLAGS) @PYTHON3_LIBS@
libPyLogbook_la_SOURCES=PyLogbook.cpp


include_HEADERS=LogBook.h LogBookPerson.h LogBookShift.h LogBookRun.h \
	LogBookNote.h

install-exec-hook:
	$(mkinstalldirs) @prefix@/pythonLibs @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/LogBook
	$(INSTALL_PROGRAM) @libdir@/libPyLogbook.so @prefix@/pythonLibs/nscldaq/LogBook/LogBook.so



#----------------------- tests ------------------------------------

check_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp logtestutils.hpp excepttests.cpp \
	bookstatictests.cpp logbooktests.cpp persontests.cpp \
	personapitest.cpp shifttests.cpp shiftapitest.cpp runtests.cpp \
	runapitests.cpp notetests.cpp noteapitests.cpp

unittests_CXXFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base/testutils \
	$(libLogbook_la_CXXFLAGS)
unittests_LDFLAGS=libLogbook.la $(libLogbook_la_LDFLAGS) @CPPUNIT_LDFLAGS@
unittests_DEPENDENCIES=libLogbook.la

# Suport for python tests:


check-TESTS:
	PYTHONPATH=@prefix@/pythonLibs/nscldaq @PYTHON@ logbookstatictests.py
	@builddir@/unittests
