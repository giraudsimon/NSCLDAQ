bin_PROGRAMS =  ringselector

ringselector_SOURCES 	= 	ringselector.cpp RingSelectorMain.cpp \
				RingBufferQueue.cpp COutputter.cpp

nodist_ringselector_SOURCES=parser.c parser.h


noinst_HEADERS		=	RingSelectorMain.h \
				RingBufferQueue.h COutputter.h

BUILT_SOURCES	= parser.h parser.c

ringselector_CPPFLAGS		= 	@LIBTCLPLUS_CFLAGS@	\
				-I@top_srcdir@/utilities/common	\
				-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/daq/format	\
			        -I@top_srcdir@/base/dataflow \
				-I@top_srcdir@/base/os	\
				-I@top_srcdir@/base/thread @PIXIE_CPPFLAGS@

ringselector_LDADD	=  	@top_builddir@/utilities/common/libUtilCommon.la	\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/os/libdaqshm.la		\
				@top_builddir@/base/thread/libdaqthreads.la     \
				@LIBEXCEPTION_LDFLAGS@				\
				$(THREADLD_FLAGS) 

ringselector_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)



#----------------------- Gengetopt stuff:

parser.c: parser.h

parser.h: parser.ggo
	$(GENGETOPT) <@srcdir@/parser.ggo --file=parser 


#----------------------- extra files to distribute.

EXTRA_DIST	=	ringselector.xml parser.ggo Asserts.h

#----------------------- Tests:
#  Since this is the first guy written to use it, the tests for
#  StringsToInteger are here too.
#

check_PROGRAMS	=	stringstointtests selectortests

stringstointtests_SOURCES	=	TestRunner.cpp str2int.cpp 

stringstointtests_CPPFLAGS	= @LIBTCLPLUS_CFLAGS@ \
				-I@top_srcdir@/utilities/common \
				-I@top_srcdir@/daq/format @PIXIE_CPPFLAGS@

stringstointtests_LDADD		= -L$(libdir)			\
				@top_builddir@/utilities/common/libUtilCommon.la \
				$(CPPUNIT_LDFLAGS)		\
				@LIBEXCEPTION_LDFLAGS@

stringstointtests_LDFLAGS	=	-Wl,"-rpath-link=$(libdir)"

selectortests_SOURCES	=	TestRunner.cpp selectortests.cpp

selectortests_LDADD	=	$(CPPUNIT_LDFLAGS)	\
				@top_builddir@/utilities/common/libUtilCommon.la	\
				@top_builddir@/base/os/libdaqshm.la \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la			\
				@LIBEXCEPTION_LDFLAGS@

selectortests_LDFLAGS	=	-Wl,"-rpath=link=$(libdir)"

selectortests_CPPFLAGS	=	$(ringselector_CPPFLAGS) \
				-DBINDIR="\"$(bindir)\"" @PIXIE_CPPFLAGS@


TESTS=./stringstointtests ./selectortests

