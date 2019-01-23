
lib_LTLIBRARIES = libSwTrigger.la

libSwTrigger_la_SOURCES = ProcessingElement.cpp ZMQContext.cpp 

libSwTrigger_la_CXXFLAGS=-I@top_srcdir@/base/thread
libSwTrigger_la_LDFLAGS=@top_builddir@/base/thread/libdaqthreads.la @ZMQ_LDFLAGS@


include_HEADERS=Worker.h MessageTypes.h  ZMQContext.h zhelpers.hpp \
	ProcessingElement.h zmqServices.h

noinst_HEADERS=zmqServices.h 
