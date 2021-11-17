PKGDIR = @prefix@/TclLibs/SBSVme

lib_LTLIBRARIES      = libSBSVme.la
libSBSVme_la_SOURCES =  VMECommand.cpp VMEMapCommand.cpp VMEPackage.cpp

libSBSVme_la_LDFLAGS =  	-version-info 1:0:0 -Wl,"-rpath=$(libdir)"

libSBSVme_la_LIBADD = @LIBTCLPLUS_LDFLAGS@ \
		@top_builddir@/sbs/nsclapi//libSBSVmeAPI.la       \
		@top_builddir@/base/CopyrightTools/libLicense.la     \
		$(TCL_LDFLAGS) 					\
		@top_builddir@/sbs/driver/src/libbtp.la  -lpthread \
		-lm -lstdc++ -lgcc -lc



libSBSVme_la_CPPFLAGS = -I@top_srcdir@/base/headers  \
	-I@top_srcdir@/sbs/nsclapi @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/CopyrightTools \
	$(TCL_FLAGS) -DHOME=\"@prefix@\" @PIXIE_CPPFLAGS@


noinst_HEADERS = VMECommand.h VMEMapCommand.h


install-exec-local:
	$(mkinstalldirs) $(PKGDIR)
	$(INSTALL_PROGRAM) .libs/libSBSVme.a $(PKGDIR)
	$(INSTALL_PROGRAM) .libs/libSBSVme.so* $(PKGDIR)
	echo "package ifneeded Vme 2.0 [list load [file join \$$dir libSBSVme.so]]" >> $(PKGDIR)/pkgIndex.tcl

EXTRA_DIST=vmetcl.xml
