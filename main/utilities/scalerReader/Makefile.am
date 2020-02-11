bin_PROGRAMS		=	scalerReader

scalerReader_SOURCES	=	scalerReader.cpp scalerReaderMain.cpp

noinst_HEADERS          =       scalerReaderMain.h

COMPILATION_FLAGS	=	-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/daq/format	\
                                -I@top_srcdir@/daq/IO	\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/servers/portmanager \
				-I@top_srcdir@/base/os


scalerReader_CPPFLAGS=$(COMPILATION_FLAGS)

scalerReader_LDADD	=	@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/daq/format/libdataformat.la	\
                                @top_builddir@/daq/IO/libdaqio.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/base/os/libdaqshm.la    \
				@LIBEXCEPTION_LDFLAGS@	\
				$(THREADLD_FLAGS)

scalerReader_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

