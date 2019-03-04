install-exec-local:
	$(mkinstalldirs) @prefix@/tkbufdump
	for f in *.tcl; do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/tkbufdump; done
	for f in *.html; do $(INSTALL_DATA)   @srcdir@/$$f @prefix@/tkbufdump; done
	$(INSTALL_DATA)   @srcdir@/packets.def @prefix@/tkbufdump
	$(mkinstalldirs) @prefix@/bin
	echo "#!/bin/bash"  > @prefix@/bin/tkdumper
	echo "@prefix@/tkbufdump/tkbufdump.tcl" >> @prefix@/bin/tkdumper
	chmod a+x @prefix@/bin/tkdumper

EXTRA_DIST = acceptall.tcl	\
controlformatter.tcl		\
datasource.tcl			\
extend.html			\
filter.html			\
intro.html			\
itemdefs.tcl			\
itemtypefilter.tcl		\
menus.html			\
next.html			\
open.html			\
packet.html			\
physicscountformatter.tcl	\
physicsformatter.tcl		\
plugin.html			\
sampleformatter.tcl		\
scalerformatter.tcl		\
stringlistformatter.tcl		\
tkbufdump.tcl			\
typefilterdialog.tcl		\
unknownformatter.tcl		\
packets.def			\
tkbufdump.xml

