lib_LTLIBRARIES=libSqlite3pp.la

libSqlite3pp_la_SOURCES=CSqlite.cpp CSqliteException.cpp CSqliteStatement.cpp \
	CSqliteTransaction.cpp CSqliteWhere.cpp
libSqlite3pp_la_CXXFLAGS=@SQLITE3_CFLAGS@ -g
libSqlite3pp_la_LDFLAGS=@SQLITE3_LIBS@

include_HEADERS=CSqlite.h CSqliteException.h CSqliteStatement.h \
	 CSqliteTransaction.h CSqliteWhere.h



noinst_PROGRAMS=unittests

unittests_SOURCES = TestRunner.cpp exceptionTest.cpp connectionTest.cpp \
			statementTest.cpp transactionTest.cpp \
			$(libSqlite3pp_la_SOURCES)

unittests_CXXFLAGS=@SQLITE3_CFLAGS@ @CPPUNIT_CFLAGS@ -g -I@top_srcdir@/base/testutils
unittests_LDFLAGS=@SQLITE3_LIBS@ @CPPUNIT_LIBS@ -g
