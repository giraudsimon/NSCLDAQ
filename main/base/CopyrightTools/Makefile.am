lib_LTLIBRARIES = libLicense.la
libLicense_la_SOURCES = CopyrightNotice.cpp

COMPILATION_FLAGS =  -I@top_srcdir@/base/headers @PIXIE_CPPFLAGS@
libLicense_la_CPPFLAGS = $(COMPILATION_FLAGS)


include_HEADERS = CopyrightNotice.h
libLicense_la_LDFLAGS =  -version-info $(SOVERSION) \
	-Wl,"-rpath-link=$(libdir)"	\
	$(THREADLD_FLAGS)
libLicense_la_CXXLFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

TESTS = ./test

noinst_PROGRAMS = test

test_SOURCES       = test.cpp

test_LDADD = libLicense.la

test_CPPFLAGS=$(COMPILATION_FLAGS)


EXTRA_DIST = copyrightnotice.xml
