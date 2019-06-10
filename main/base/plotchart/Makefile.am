#
# Automake file to install plotchart
#
# For now this is installing Arjen's Plotchart 2.0 provisional which includes
# a pile of stuff he did for me that has not made it into linux releases
# If newer releases make it in, they'll get preferentially used given the
# way package require operates.

destdir=@prefix@/TclLibs/plotchart
files=pkgIndex.tcl xyplot.tcl plotanim.tcl plotannot.tcl plotaxis.tcl plotbind.tcl \
    plotbusiness.tcl plotchart.tcl plotcombined.tcl plotconfig.tcl plotcontour.tcl \
    plotgantt.tcl plotobject.tcl plotpack.tcl plotpriv.tcl plotscada.tcl plotspecial.tcl \
    plot3d.tcl plottable.tcl scaling.tcl


install-exec-local:
	$(mkinstalldirs) $(destdir)
	for f in $(files); do @INSTALL_SCRIPT@ @srcdir@/$$f $(destdir); done


EXTRA_DIST=$(files) plotchart.xml

