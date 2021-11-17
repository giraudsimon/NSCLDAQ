bin_PROGRAMS		=	sclclient

noinst_PROGRAMS         =       testData

sclclient_SOURCES	=	TclServerConnection.cpp		\
				TcpClient.cpp			\
				sclclientMain.cpp		\
				sclclient.cpp 

nodist_sclclient_SOURCES= sclclientargs.c sclclientargs.h

noinst_HEADERS		=	TclServerConnection.h		\
				TcpClient.h			\
				sclclientMain.h	



COMPILATION_FLAGS		=	-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/daq/format	\
                                -I@top_srcdir@/daq/IO	\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/servers/portmanager \
				-I@top_srcdir@/base/os @PIXIE_CPPFLAGS@


sclclient_CPPFLAGS=$(COMPILATION_FLAGS)

BUILT_SOURCES		= 	sclclientargs.c sclclientargs.h

sclclient_LDADD		=	@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/daq/format/libdataformat.la	\
                                @top_builddir@/daq/IO/libdaqio.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/base/os/libdaqshm.la    \
				@LIBEXCEPTION_LDFLAGS@	\
				$(THREADLD_FLAGS)

sclclient_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

testData_SOURCES        =       testData.cpp
testData_CXXFLAGS       =       -I@top_srcdir@/daq/format $(COMPILATION_FLAGS)
testData_LDADD          =       @top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/base/os/libdaqshm.la    \
				@LIBEXCEPTION_LDFLAGS@ 	\
				$(THREADLD_FLAGS)




sclclientargs.c: sclclientargs.h

sclclientargs.h: sclclientargs.ggo
	$(GENGETOPT) < @srcdir@/sclclientargs.ggo --file sclclientargs \
		--output-dir=@builddir@


EXTRA_DIST		= 	sclclientargs.ggo	\
				sclclient.xml
