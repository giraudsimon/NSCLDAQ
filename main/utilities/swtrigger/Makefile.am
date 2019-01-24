
lib_LTLIBRARIES = libSwTrigger.la

libSwTrigger_la_SOURCES = ProcessingElement.cpp ZMQPushDataSource.cpp \
	ZMQContext.cpp 

libSwTrigger_la_CXXFLAGS=-I@top_srcdir@/base/thread
libSwTrigger_la_LDFLAGS=@top_builddir@/base/thread/libdaqthreads.la @ZMQ_LDFLAGS@


include_HEADERS=MessageTypes.h ZMQPushDataSource.h  \
	ZMQContext.h zhelpers.hpp \
	ProcessingElement.h zmqServices.h

noinst_HEADERS=zmqServices.h 
