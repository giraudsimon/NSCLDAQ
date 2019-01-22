
lib_LTLIBRARIES = libSwTrigger.la

libSwTrigger_la_SOURCES = Worker.cpp ZMQDataSource.cpp

libSwTrigger_la_CXXFLAGS=-I@top_srcdir@/base/thread
libSwTrigger_la_LDFLAGS=@top_builddir@/base/thread/libdaqthreads.la


include_HEADERS=Worker.h MessageTypes.h ZMQDataSource.h

noinst_HEADERS=zmqServices.h 
