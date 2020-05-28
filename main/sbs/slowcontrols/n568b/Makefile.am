DRIVERDIR=@prefix@/TclLibs/v568
APPDIR=@prefix@/bin
HELPDIR=@prefix@/TclLibs/data/n568

VERSION=2.0			#Must match the package versions!!!

PACKAGES=n568b.tcl n568Panel.tcl
APPS=loadshaper.tcl n568Control.tcl

HELPFILES=gui.html overview.html close.html config.html exit.html filemenu.html \
    helpmenu.html new.html read.html write.html

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	for f in $(PACKAGES) ; do $(INSTALL_SCRIPT) @srcdir@/$$f $(DRIVERDIR); done
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f $(APPDIR)/`basename $$f .tcl`; done
	for f in $(HELPFILES); do $(INSTALL_DATA)   @srcdir@/$$f $(HELPDIR); done
	echo "pkg_mkIndex -verbose $(DRIVERDIR) *.tcl" | $(TCLSH_CMD)


EXTRA_DIST= $(PACKAGES) $(APPS) $(MANPAGES) $(HELPFILES) n568.xml

