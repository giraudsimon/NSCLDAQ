bin_PROGRAMS = vmusbcaenupgrader

vmusbcaenupgrader_SOURCES = CAENComm_VMUSB.cpp

vmusbcaenupgrader_CPPFLAGS  = -I@top_srcdir@/base/thread -I@top_srcdir@/caenfw-common \
		-I@top_srcdir@/usb/vmusb/vmusb -I@top_srcdir@/usb/usb1 \
		@libusb_1_0_CFLAGS@

vmusbcaenupgrader_LDADD = @top_builddir@/caenfw-common/libcaenfwloadCommon.la \
	@top_builddir@/usb/vmusb/vmusb/libVMUSB.la @top_builddir@/base/thread/libdaqthreads.la \
	@top_builddir@/usb/usb1/libUSB1.la @libusb_1_0_LIBS@ \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@

EXTRA_DIST =  vmusbcaenupgrader.xml
