lib_LTLIBRARIES 		= libConfigurableObject.la
libConfigurableObject_la_SOURCES	= 	XXUSBConfigurableObject.cpp
include_HEADERS	= 	XXUSBConfigurableObject.h	

libConfigurableObject_la_CXXFLAGS=@THREADCXX_FLAGS@ @TCL_FLAGS@ \
	@LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@

libConfigurableObject_la_LIBADD	= @LIBTCLPLUS_LDFLAGS@	\
					@TCL_LDFLAGS@ \
				-lm @THREADLD_FLAGS@

