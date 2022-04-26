lib_LTLIBRARIES=libUSB01.la

libUSB01_la_SOURCES=USB.cpp USBDeviceInfo.cpp USBDevice.cpp XXUSBUtil.cpp
include_HEADERS=USB.h USBDeviceInfo.h USBDevice.h XXUSBUtil.h

libUSB01_la_CXXFLAGS=-I@top_srcdir@/base/thread \
	-I@top_srcdir@/base/os \
	@libusb_CFLAGS@
libUSB01_la_LDFLAGS= @libusb_LIBS@ \
	@top_builddir@/base/thread/libdaqthreads.la \
	@top_builddir@/base/os/libdaqshm.la
