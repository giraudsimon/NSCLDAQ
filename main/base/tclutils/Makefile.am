lib_LTLIBRARIES = libWait.la
libWait_la_SOURCES = WaitPackage.cpp
libWait_la_CPPFLAGS=@TCL_CPPFLAGS@ @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/CopyrightTools @PIXIE_CPPFLAGS@

libWait_la_LDFLAGS =	@LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@	\
			@top_builddir@/base/CopyrightTools/libLicense.la	 \
			-version-info $(SOVERSION)

libWait_la_LIBADD =  @LIBTCLPLUS_LDFLAGS@ \
	$(TCL_LDFLAGS)	$(X11LIBS)  -lLicense  \
	-lm -lstdc++ -lgcc -lc



DRIVERDest=@prefix@/TclLibs/tclutils

TCLDRIVERS= Utils.tcl BlockCompleter.tcl TclSourceFilter.tcl Process.tcl \
   Orphan.tcl
TCLTESTS= utils.test blockcompleter.test tclsourcefilter.test wait.test

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

install-exec-hook:
	echo Adjusting installation location of libWait.so compiled tcl extension.
	mv $(prefix)/lib/libWait.* $(prefix)/TclLibs
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs *.so" | $(TCLSH_CMD)


DOCS=BlockCompleter.xml Process.xml TclSourceFilter.xml Utils.xml

check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs @libdir@ @srcdir@" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) tclTests.tcl $(TCLTESTS) $(DOCS)

