lib_LTLIBRARIES = libSecurity.la
libSecurity_la_SOURCES = AccessListCheck.cpp  \
	FdInteractor.cpp HostListCheck.cpp \
	IOInteractor.cpp Interactor.cpp PasswordCheck.cpp  \
	StringInteractor.cpp TclAccessListCheck.cpp UnixUserCheck.cpp
include_HEADERS = AccessListCheck.h  Authenticator.h \
	FdInteractor.h HostListCheck.h \
	IOInteractor.h Interactor.h PasswordCheck.h StringInteractor.h \
	TclAccessListCheck.h UnixUserCheck.h
libSecurity_la_LDFLAGS = @top_builddir@/base/os/libdaqshm.la \
	@LIBTCLPLUS_LDFLAGS@		\
	-version-info $(SOVERSION) -Wl,"-rpath=$(libdir)" \
		$(THREADLD_FLAGS)

COMPILATION_FLAGS = -I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/os $(TCL_FLAGS) \
	@PIXIE_CPPFLAGS@

libSecurity_la_CPPFLAGS	= $(THREADCXX_FLAGS) $(AM_CXXFLAGS) $(COMPILATION_FLAGS)




EXTRA_DIST= security.xml



