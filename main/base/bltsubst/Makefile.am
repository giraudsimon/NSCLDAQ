#
#  AutoMake input file for installing the BLT substitution widgets.
#
#  These go to @prefix@/TclLibs/blt
#
# Note for graphics, we install the RBC toolkit.
# So that we don't need the Tcl source tree to build this .so's are included
# for linux 32/64 bit systems.  The sources are available an licensed under
# the BSD license at: http://sourceforge.net/projects/rbctoolkit
#

destdir=@prefix@/TclLibs/blt
rbcdir=@prefix@/TclLibs/rbc

packages= tabset.tcl 
rbcfiles=graph.tcl pkgIndex.tcl rbcCanvEps.pro rbcGraph.pro


install-exec-local:
	$(mkinstalldirs) $(destdir)
	for f in $(packages); do @INSTALL_SCRIPT@ @srcdir@/$$f $(destdir); done
	echo pkg_mkIndex $(destdir) "*.tcl" | @TCLSH_CMD@
	$(mkinstalldirs) $(rbcdir)
	if test `getconf LONG_BIT` = "32"; then $(INSTALL_PROGRAM) @srcdir@/librbc0.1.so-32 $(rbcdir)/librbc0.1.so; else $(INSTALL_PROGRAM) @srcdir@/librbc0.1.so-64 $(rbcdir)/librbc0.1.so; fi
	for f in $(rbcfiles); do @INSTALL_SCRIPT@ @srcdir@/$$f  $(rbcdir); done

EXTRA_DIST=$(packages) $(rbcfiles) vector.test rbcCanvEps.pro \
	rbcGraph.pro librbc0.1.so-32 librbc0.1.so-64