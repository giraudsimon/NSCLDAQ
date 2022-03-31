lib_LTLIBRARIES=libUSB1.la

libUSB1_la_SOURCES=USB.cpp USBDeviceInfo.cpp USBDevice.cpp XXUSBUtil.cpp
include_HEADERS=USB.h USBDeviceInfo.h USBDevice.h XXUSBUtil.h

libUSB1_la_CXXFLAGS=-I@top_srcdir@/base/thread \
	-I@top_srcdir@/base/os \
	@libusb_1_0_CFLAGS@
libUSB1_la_LDFLAGS= @libusb_1_0_LIBS@ \
	@top_builddir@/base/thread/libdaqthreads.la \
	@top_builddir@/base/os/libdaqshm.la
