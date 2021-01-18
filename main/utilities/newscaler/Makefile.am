# The pure TCl parts of this.

PACKAGE_FILES=nameMap.tcl channel.tcl scalerconfig.tcl page.tcl \
	singleModel.tcl ratioModel.tcl scalermain.tcl header.tcl reports.tcl\
	scalerUtil.tcl stripparam.tcl stripratio.tcl emptyModel.tcl zoomPrompt.tcl
TEST_FILES=nameMap.test channel.test channelcmd.test singleModel.test \
	ratioModel.test stripparam.test stripratio.test emptyModel.test tclTests.tcl
APP_FILES=ScalerDisplay.tcl.in
IMAGES=red.gif green.gif amber.gif

DOCS=scalerdisplay_user.xml scalerdisplay_man.xml \
		 scalerdisplay_user_bookinfo.xml tclring_user.xml
FIGURES = @srcdir@/figures/ScalerDisplayUserGuide_empty_600.png \
					 @srcdir@/figures/ScalerDisplayUserGuide_data_600.png \
					 @srcdir@/figures/ScalerDisplayUserGuide_stripchart_600.png \
					 @srcdir@/figures/ScalerDisplayUserGuide_GimpStripChart_600.png

# auto book
BOOK=scalerdisplay_user_autobook.pdf
CHAPTER2BOOK = @PYTHON@ @top_srcdir@/docbuild/chapter2book.py

INTERACTIVE_TESTS=pagetest.tcl 




INSTDIR=@prefix@/TclLibs/newscaler
GRAPHDIR=@prefix@/share/scalerdisplay
FIGUREDIR=@prefix@/share/html/figures

install-exec-hook:
	$(mkinstalldirs) $(INSTDIR) 
	$(mkinstalldirs) $(GRAPHDIR)
	$(mkinstalldirs) $(FIGUREDIR)
	$(mkinstalldirs) @prefix@/share/pdfs
	install @libdir@/libTclRingBuffer.so $(INSTDIR)
	(cd @srcdir@; $(INSTALL_SCRIPT) $(PACKAGE_FILES) $(INSTDIR))
	echo pkg_mkIndex -verbose $(INSTDIR) "*.tcl" | @TCLSH_CMD@
	echo "package ifneeded TclRingBuffer 1.0 [list load [file join \$$dir libTclRingBuffer.so]]" >> $(INSTDIR)/pkgIndex.tcl
	install @builddir@/ScalerDisplay.tcl @bindir@/ScalerDisplay
	$(CHAPTER2BOOK) -s @srcdir@/scalerdisplay_user.xml -b @srcdir@/scalerdisplay_user_bookinfo.xml -o .
	$(INSTALL_DATA)  $(BOOK) @prefix@/share/pdfs
	(cd @srcdir@; install $(IMAGES) $(GRAPHDIR))
	install $(FIGURES) $(FIGUREDIR)



check-TESTS: check_PROGRAMS
	@TCLSH_CMD@ @srcdir@/tclTests.tcl


EXTRA_DIST=$(PACKAGE_FILES) $(TEST_FILES) $(INTERACTIVE_TESTS) $(APP_FILES) $(IMAGES) \
	$(DOCS) $(FIGURES)

#  The Tcl API to get data from rings...and tests.

lib_LTLIBRARIES=libTclRingBuffer.la

libTclRingBuffer_la_SOURCES=CTclRingCommand.cpp
libTclRingBuffer_la_LIBADD=	@top_builddir@/daq/format/libdataformat.la     \
				@top_builddir@/base/dataflow/libDataFlow.la    \
				@top_builddir@/base/tcpip/libTcp.la		\
				@top_builddir@/base/os/libdaqshm.la 		\
				@LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@ @THREADLD_FLAGS@


libTclRingBuffer_la_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ @THREADCXX_FLAGS@ \
				-I@top_srcdir@/base/dataflow \
				-I@top_srcdir@/daq/format \
				-I@top_srcdir@/base/os 

noinst_HEADERS=CTclRingCommand.h

noinst_PROGRAMS   = ringTests
ringTests_SOURCES = TestRunner.cpp ringTests.cpp
ringTests_CPPFLAGS = @LIBTCLPLUS_CFLAGS@ @CPPUNIT_CFLAGS@ @TCL_FLAGS@ @THREADCXX_FLAGS@ \
			-I@top_srcdir@/daq/format -I@top_srcdir@/base/dataflow
ringTests_LDADD    =  @CPPUNIT_LDFLAGS@ libTclRingBuffer.la \
			@top_builddir@/daq/format/libdataformat.la     \
				@top_builddir@/base/dataflow/libDataFlow.la    \
				@LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@ @THREADLD_FLAGS@


TESTS=ringTests
check_PROGRAMS:  
	@builddir@/ringTests
