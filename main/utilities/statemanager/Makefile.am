PACKAGE_FILES = StateMachine.py GenericState.py  Utilities.py StateMonitor.py

MAINS = stateManager.py

INSTDIR = @prefix@/pythonLibs/nscldaq/statemanager
TCLLIBDIR=@prefix@/TclLibs

FIGURES=statediagram.jpg

##
# installation hook: create the installation directory tree.
#                    install the packges in  INSTDIR
#                    ensure there's an __init__.py which is required for
#                    package directories.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	$(mkinstalldirs)  @datarootdir@/html
	touch $(INSTDIR)/__init__.py 
	for f in $(MAINS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .py`; done
	for f in  $(PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(INSTDIR); done
	$(mkinstalldirs) $(TCLLIBDIR)/statemanager
	echo package ifneeded statemanager 1.0 [list load [file join @libdir@ libStateMonitor.so]] > $(TCLLIBDIR)/statemanager/pkgIndex.tcl
	for f in $(FIGURES); do $(INSTALL_DATA) @srcdir@/$$f @datarootdir@/html; done



EXTRA_DIST=$(PACKAGE_FILES) $(MAINS) $(FIGURES) \
	MonitorTest.py CommandTest.py StateMonitorTest1.py tcltest1.tcl statemanager.xml


## 
# Bindings for C++





lib_LTLIBRARIES = libStateMonitor.la

libStateMonitor_la_SOURCES=CStateMonitor.cpp CTCLStateMonitor.cpp CStateMonitorThread.cpp
libStateMonitor_ladir     = @includedir@ -I@top_srcdir@/base/thread @THREADCXX_FLAGS@
libStateMonitor_la_HEADERS= CStateMonitor.h CTCLStateMonitor.h  CStateMonitorThread.h
libStateMonitor_la_LIBADD =  @top_builddir@/base/thread/libdaqthreads.la \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LIBS@ @ZMQ_LDFLAGS@ @THREADLD_FLAGS@
libStateMonitor_la_CPPFLAGS = @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@ -I@top_srcdir@/base/thread

noinst_PROGRAMS=Polltest StateMonitorBaseTest StateMonitorTest tclTest1 ThreadTest
Polltest_SOURCES=Polltest.cpp
Polltest_LDFLAGS=libStateMonitor.la @ZMQ_LDFLAGS@
Polltest_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@ -I@top_srcdir@/base/thread

StateMonitorBaseTest_SOURCES=CStateMonitorBaseTest.cpp
StateMonitorBaseTest_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ @ZMQ_LDFLAGS@
StateMonitorBaseTest_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@ -I@top_srcdir@/base/thread

StateMonitorTest_SOURCES=CStateMonitorTest.cpp
StateMonitorTest_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ @ZMQ_LDFLAGS@
StateMonitorTest_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@ -I@top_srcdir@/base/thread

tclTest1_SOURCES=tclTest1.cpp
tclTest1_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ @ZMQ_LDFLAGS@ @TCL_LIBS@ -Wl,"-rpath=@libdir@"
tclTest1_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@ -I@top_srcdir@/base/thread

ThreadTest_SOURCES=ThreadTest.cpp
ThreadTest_LDFLAGS=libStateMonitor.la @top_builddir@/base/thread/libdaqthreads.la @LIBTCLPLUS_LDFLAGS@ @ZMQ_LDFLAGS@ @TCL_LIBS@ -Wl,"-rpath=@libdir@"
ThreadTest_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@ -I@top_srcdir@/base/thread