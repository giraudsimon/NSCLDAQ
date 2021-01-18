
if EPICS

lib_LTLIBRARIES=libEpicslib.la

libEpicslib_la_SOURCES=epicslib.cpp epicslib.h

libEpicslib_la_CXXFLAGS=$(THREADCXX_FLAGS) $(AM_CXXFLAGS) $(EPICS_BIN) $(EPICS_INCLUDES) \
	-I@top_srcdir@/Headers \
        -I@top_srcdir@/base/os


libEpicslib_la_LIBADD= @top_builddir@/base/os/libdaqshm.la \
	$(EPICS_LDFLAGS) $(THREADLD_FLAGS) @TCL_LDFLAGS@

endif
