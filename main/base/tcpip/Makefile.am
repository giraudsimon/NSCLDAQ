lib_LTLIBRARIES = libTcp.la

libTcp_la_SOURCES = CSocket.cpp \
	CTCPBadSocketState.cpp \
	CTCPConnectionFailed.cpp \
	CTCPConnectionLost.cpp \
	CTCPNoSuchHost.cpp \
	CTCPNoSuchService.cpp

include_HEADERS = CSocket.h \
	CTCPBadSocketState.h \
	CTCPConnectionFailed.h \
	CTCPConnectionLost.h \
	CTCPNoSuchHost.h \
	CTCPNoSuchService.h


libTcp_la_CPPFLAGS = @LIBTCLPLUS_CFLAGS@	\
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/os @PIXIE_CPPFLAGS@

libTcp_la_LIBADD = @LIBEXCEPTION_LDFLAGS@	\
	@top_builddir@/base/os/libdaqshm.la

EXTRA_DIST=Tcp.xml
