bin_PROGRAMS = compatibilitybuffer compatibilitylogger BufferToRing convert10to11

noinst_HEADERS = buffer.h buftypes.h CBufferConverter.h CLogRun.h \
    OldDataFormat.h



compatibilitybuffer_SOURCES = compatibilitybuffer.cpp CBufferConverter.cpp 
nodist_compatibilitybuffer_SOURCES = parser.c parser.h

EXTRA_DIST=compatibilitybuffer.ggo loggeroptions.ggo eventlog-compat.in \
	spectcldaq.in spectcldaq.server.tcl compatibility.xml s800toring.in \
	netcat.tcl buffertoring.ggo test.ggo


compatibilitybuffer_LDADD = \
	@top_builddir@/daq/format/libdataformat.la
compatibilitybuffer_CPPFLAGS = -I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os @PIXIE_CPPFLAGS@

compatibilitylogger_SOURCES = compatibilitylogger.cpp CLogRun.cpp 
nodist_compatibilitylogger_SOURCES = loggeroptions.c loggeroptions.h
compatibilitylogger_CPPFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os @PIXIE_CPPFLAGS@



BufferToRing_SOURCES = BufferToRing.cpp 
nodist_BufferToRing_SOURCES = btoroptions.c btoroptions.h

BufferToRing_LDADD   = @top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/os/libdaqshm.la
BufferToRing_LDFLAGS = -ldl
BufferToRing_CPPFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os -I@top_srcdir@/daq/eventbuilder @PIXIE_CPPFLAGS@


convert10to11_SOURCES = Convert10to11.cpp

convert10to11_LDADD = @top_builddir@/daq/format/libdataformat.la \
    @top_builddir@/base/os/libdaqshm.la \
                @top_builddir@/daq/IO/libdaqio.la
convert10to11_CPPFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os \
        -I@top_srcdir@/daq/IO @PIXIE_CPPFLAGS@


#  Anything that is generated from a .in file is in builddir anything
#  else is in srcdir:

install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) @builddir@/eventlog-compat $(bindir)
	$(INSTALL_SCRIPT) @builddir@/spectcldaq $(bindir)
	$(INSTALL_SCRIPT) @srcdir@/spectcldaq.server.tcl  $(bindir)/spectcldaq.server
	$(INSTALL_SCRIPT) @builddir@/s800toring  $(bindir)
	$(INSTALL_SCRIPT) @srcdir@/netcat.tcl $(bindir)
	$(INSTALL_SCRIPT) @builddir@/bufdump $(bindir)


#----------------------------------------------------------------

#  This stuff is needed to be sure that gengetopt builds the stuff 
#  we need prior to application compilations.


BUILT_SOURCES = parser.c parser.h  loggeroptions.c loggeroptions.h \
	btoroptions.c btoroptions.h  test.c test.h

parser.c: parser.h

parser.h: compatibilitybuffer.ggo
	$(GENGETOPT) <@srcdir@/compatibilitybuffer.ggo --file=parser \
		--output-dir=@builddir@


loggeroptions.c: loggeroptions.h

loggeroptions.h: loggeroptions.ggo
	$(GENGETOPT) <@srcdir@/loggeroptions.ggo --file=loggeroptions \
		--output-dir=@builddir@


btoroptions.c: btoroptions.h

btoroptions.h: buffertoring.ggo
	$(GENGETOPT) <@srcdir@/buffertoring.ggo --file=btoroptions \
		--output-dir=@builddir@


#------------------------------------------------------------------------
# For testing:
#

noinst_PROGRAMS = testDataSource

testDataSource_SOURCES = testDataSource.cpp 
nodist_testDataSource_SOURCES = test.c test.h

testDataSource_CPPFLAGS = -I@top_srcdir@/base/os @PIXIE_CPPFLAGS@


testDataSource_LDADD    = @top_builddir@/base/os/libdaqshm.la \
	@LIBTCLPLUS_LDFLAGS@  @TCL_LIBS@

test.c: test.h

test.h: test.ggo
	$(GENGETOPT) <@srcdir@/test.ggo --file=test \
		--output-dir=@builddir@
