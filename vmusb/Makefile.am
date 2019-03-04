lib_LTLIBRARIES		= libVMUSB.la
libVMUSB_la_SOURCES 	= CVMUSB.cpp CVMUSBReadoutList.cpp
include_HEADERS		= CVMUSB.h   CVMUSBReadoutList.h 
INCLUDES		= $(USBSWITCHES)

libVMUSB_la_LIBADD       = $(USB_LIBS)

#IVP tests:


TEST_MODULES = TestRunner.cpp  regTests.cpp


tests: $(TEST_MODULES)
	$(CXX) -g -o tests -I. -I$(prefix)/include $(CPPUNIT_INCLUDES) \
	$(TEST_MODULES) $(CPPUNIT_LDFLAGS) -L$(prefix)/lib -lVMUSB \
	-Wl,"-rpath=$(prefix)/lib"

installcheck-local: tests
	./tests

