bin_PROGRAMS = tclserver
tclserver_SOURCES = TclAuthorizer.cpp serverinstance.cpp tclAppInit.cpp \
	tkAppInit.cpp Server.cpp

tclserver_LDADD = @LIBTCLPLUS_LDFLAGS@ \
	 @top_builddir@/servers/portmanager/libPortManager.la \
	 @top_builddir@/base/CopyrightTools/libLicense.la    \
	 @top_builddir@/base/os/libdaqshm.la	\
	@LIBEXCEPTION_LDFLAGS@	\
	$(TCL_LDFLAGS) \
	$(X11LIBS)  	\
	$(THREADLD_FLAGS)

tclserver_CPPFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS) @PIXIE_CPPFLAGS@ \
	-I@top_srcdir@/base/headers \
        -I@top_srcdir@/base/exception \
        -I@top_srcdir@/servers/portmanager -I@top_srcdir@/base/os \
        @PIXIE_CPPFLAGS@ \
        @LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/CopyrightTools $(TCL_FLAGS)


tclserver_LDFLAGS=-Wl,"-rpath=$(libdir)"


noinst_HEADERS = TclAuthorizer.h \
	server.h \
	serverinstance.h


EXTRA_DIST = tclserver.xml serverPackage.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/tclserver
	$(INSTALL_SCRIPT) @srcdir@/serverPackage.tcl @prefix@/TclLibs/tclserver
	echo "pkg_mkIndex @prefix@/TclLibs/tclserver *.tcl" | @TCLSH_CMD@
