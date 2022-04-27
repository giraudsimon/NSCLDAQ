SUBDIRS = 	ccusb 		\
		daqconfig	\
		ctlconfig \
		core \
		driverkit   \
		tcldrivers \
		controlpanels \
		tclpackage \
		devtests \
		tclservertests


bin_PROGRAMS = CCUSBReadout

CCUSBReadout_SOURCES = CTheApplication.cpp 
nodist_CCUSBReadout_SOURCES=  cmdline.c cmdline.h

noinst_HEADERS = CTheApplication.h 

CCUSBReadout_CPPFLAGS = -I@top_srcdir@/usb/ccusb/ccusb \
	 -I@top_srcdir@/usb/ccusb/daqconfig \
	 -I@top_srcdir@/usb/ccusb/ctlconfig \
	 -I@top_srcdir@/usb/ccusb/core \
	-I@top_srcdir@/usb/common/slowcontrols  \
	-I@top_srcdir@/usb/common/configurableobject \
	-I@top_srcdir@/usb/usb0.1         \
	-I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/thread \
	-I@top_srcdir@/base/dataflow	                       \
	-I@top_srcdir@/base/os				\
        -I@top_srcdir@/servers/portmanager \
	$(TCL_FLAGS) @THREADCXX_FLAGS@ @libusb_CFLAGS@ @PIXIE_CPPFLAGS@


CCUSBReadout_LDADD	= \
	@top_builddir@/usb/ccusb/ccusb/libCCUSB.la		\
	@top_builddir@/usb/ccusb/daqconfig/libCCUSBDaqConfig.la		\
	@top_builddir@/usb/ccusb/ctlconfig/libCCUSBCtlConfig.la		\
	@top_builddir@/usb/ccusb/core/libCCUSBCore.la \
	@top_builddir@/usb/usb0.1/libUSB01.la   \
	@top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@top_builddir@/base/os/libdaqshm.la \
	@LIBTCLPLUS_LDFLAGS@	\
	@TCL_LDFLAGS@ @THREADLD_FLAGS@ @libusb_LIBS@

CCUSBReadout_LDFLAGS=-Wl,"-rpath=$(libdir)"

CCUSBReadout_CXXFLAGS= -g

BUILT_SOURCES = cmdline.c cmdline.h

cmdline.c: cmdline.h

cmdline.h: commandline.ggo
	@GENGETOPT@ < @srcdir@/commandline.ggo --output-dir=@builddir@ --set-version=@VERSION@

EXTRA_DIST= commandline.ggo ccusbReadout_user.xml ccusbReadout_man.xml

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/include
	$(INSTALL_DATA)  @srcdir@/*.h  @prefix@/ccusbdriver/include
