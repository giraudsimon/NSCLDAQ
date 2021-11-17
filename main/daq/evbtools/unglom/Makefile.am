bin_PROGRAMS=unglom

unglom_SOURCES=unglom.cpp @srcdir@/cmdline.c

unglom_CPPFLAGS=-I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format       \
	@LIBTCLPLUS_CFLAGS@	\
	-I@top_srcdir@/base/os		\
	-I@top_srcdir@/base/headers   @PIXIE_CPPFLAGS@

unglom_LDADD = @top_builddir@/daq/format/libdataformat.la  \
	@top_builddir@/daq/eventbuilder/libEventBuilder.la \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBTCLPLUS_LDFLAGS@

BUILT_SOURCES = @srcdir@/cmdline.c @srcdir@/cmdline.h

@srcdir@/cmdline.c: @srcdir@/cmdline.h

@srcdir@/cmdline.h: @srcdir@/cmdline.ggo
	@GENGETOPT@ --input=@srcdir@/cmdline.ggo \
		--output-dir=@srcdir@

EXTRA_DIST=cmdline.ggo unglom.xml
