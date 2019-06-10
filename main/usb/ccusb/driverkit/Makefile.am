install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA) CCUSBDriverIncludes @srcdir@/drivertemplate.cpp @prefix@/ccusbdriver
	$(INSTALL_DATA) Makefile-template @prefix@/ccusbdriver/Makefile

EXTRA_DIST=drivertemplate.cpp Makefile-template.in CCUSBDriverIncludes.in