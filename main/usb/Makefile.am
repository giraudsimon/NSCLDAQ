SUBDIRS=common vmusb ccusb loaders caenfwloader mesytec

DIST_SUBDIRS = common vmusb ccusb loaders caenfwloader mesytec usb1 usb0.1

#
#  Index the swig wrapper for the XX_USB packages:
#  This must be done manually as pkg_mkIndex failes if there are no controllers online:
install-exec-local:
	rm -f @libdir@/pkgIndex.tcl
	echo package ifneeded cccusb 1.0 [list load [file join \$$dir libCCCUSB.so]] >> @libdir@/pkgIndex.tcl
	echo package ifneeded cccusbreadoutlist 1.0 [list load [file join \$$dir libCCCUSBReadoutList.so]] >> @libdir@/pkgIndex.tcl
	echo package ifneeded cvmusb 1.0 [list load [file join \$$dir libCVMUSB.so]] >> @libdir@/pkgIndex.tcl
	echo package ifneeded cvmusbreadoutlist 1.0 [list load [file join \$$dir libCVMUSBReadoutList.so]] >> @libdir@/pkgIndex.tcl

