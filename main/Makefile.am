ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS=subdir-objects




if CAENSUPPORT
CAENSUPPORTDIRS=caendigitizers
endif

if BUILD_EPICS_TOOLS
if EPICS
     EPICSDIRS=epics
endif
endif

if BUILD_USB
   USBDIR=usb
endif

if BUILD_DOCS
   DOCDIR=docbuild docconfig simplesetups cookbooks
endif

if BUILD_DDAS
  DDASDIR=ddas
endif

SUBDIRS =	 base caenfw-common servers  base/dataflow  \
		daq utilities $(DOCDIR) \
		sbs $(EPICSDIRS) $(USBDIR) $(DDASDIR) $(CAENSUPPORTDIRS) examples

DIST_SUBDIRS = base caenfw-common servers base/dataflow daq utilities \
		 docbuild docconfig simplesetups sbs epics usb ddas  \
		caendigitizers    \
		examples

INTROFIGURES = @srcdir@/figures/intro-data-flow-diagram.png \
								@srcdir@/figures/intro-data-flow-diagram-diagnostics.png 
install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @includedir@
	$(mkinstalldirs) @prefix@/pythonLibs @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/share/html/figures
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
	$(INSTALL_DATA) config.h @srcdir@/ddas/config_pixie16api.h @includedir@
	touch @prefix@/pythonLibs/nscldaq/__init__.py
	echo "Version: "NSCLDAQ-$(VERSION) built on `hostname` `date` > @prefix@/VERSION
	$(INSTALL_DATA) @srcdir@/ChangeLog @prefix@
	$(INSTALL_SCRIPT) daqsetup.bash @prefix@
	$(INSTALL_DATA) @srcdir@/figures/*.png @prefix@/share/html/figures


EXTRA_DIST=dotests introduction.xml libtcl $(INTROFIGURES) cookbooks ddas/config_pixie16api.h

check-TESTS:
	@top_srcdir@/dotests $(SUBDIRS)
