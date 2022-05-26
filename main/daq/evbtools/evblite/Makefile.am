bin_PROGRAMS=evbtagger

evbtagger_SOURCES=evbtagger.cpp evbtagger.h evbtaggerMain.cpp ItemTagger.h ItemTagger.cpp

evbtagger_CPPFLAGS=-I@top_srcdir@/base/os -I@top_srcdir@/daq/eventbuilder -I@top_srcdir@/daq/format
evbtagger_LDFLAGS=@top_builddir@/base/os/libdaqshm.la \
	@top_builddir@/daq/format/libdataformat.la

BUILT_SOURCES=evbtagger.cpp evbtagger.h

evbtagger.cpp: evbtagger.h

evbtagger.h: evbtagger.ggo
	$(GENGETOPT) <@srcdir@/evbtagger.ggo -c cpp -F evbtagger

install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs/evb-lite
	$(INSTALL_SCRIPT) @srcdir@/evblitePackage.tcl @prefix@/TclLibs/evb-lite
	echo "pkg_mkIndex -verbose @prefix@/TclLibs/evb-lite *.tcl" | @TCLSH_CMD@


EXTRA_DIST=evblitePackage.tcl evbtagger.ggo
