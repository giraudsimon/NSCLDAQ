# Generated automatically from Makefile.in by configure.
# Makefile.in --
#
#	This file is a Makefile for the tclhttpd web server
#
# Copyright (c) 1999-2000 Scriptics Corporation.
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# RCS: @(#) $Id: Makefile.in,v 1.30 2003/08/11 16:46:57 welch Exp $

#========================================================================
# Edit the following few lines when writing a new extension
#========================================================================

#========================================================================
# Change the name of the variable "exampleA_LIB_FILE" to match the one
# used in the configure script.  This is the parameterized name of the
# library that we are building.
#========================================================================

#lib_BINARIES=$(crypt_LIB_FILE) $(limit_LIB_FILE)
lib_BINARIES=$(crypt_LIB_FILE) $(limit_LIB_FILE)
BINARIES=$(lib_BINARIES)

#========================================================================
# Enumerate the names of the source files included in this package.
# This will be used when a dist target is added to the Makefile.
#========================================================================

limit_SOURCES = src/limit.c
crypt_SOURCES = src/crypt.c src/cryptLib.c
SOURCES = $(crypt_SOURCES) $(limit_SOURCES)

#========================================================================
# Enumerate the names of the object files included in this package.
# These objects are created and linked into the final library.  In
# most cases these object files will correspond to the source files
# above.
#
# CRYPT_OBJS is the name of the object files needed to provide the
# crypt() C function.  Most unix platforms already have it in a standard
# library.  Windows doesn't, so we need to account for that possibility
# here.
#========================================================================

CRYPT_OBJS = 
limit_OBJECTS =  limit.$(OBJEXT)
crypt_OBJECTS =  crypt.$(OBJEXT) $(CRYPT_OBJS)
OBJECTS = $(crypt_OBJECTS)

#========================================================================
# The substitution of "exampleA_LIB_FILE" into the variable name below
# allows us to refer to the objects for the library without knowing the name
# of the library in advance.  It also lets us use the "$@" variable in
# the rule for building the library, so we can refer to both the list of 
# objects and the library itself in a platform-independent manner.
#========================================================================

limit_LIB_FILE = liblimit1.0.so
$(limit_LIB_FILE)_OBJECTS = $(limit_OBJECTS)

crypt_LIB_FILE = libcrypt1.0.so
$(crypt_LIB_FILE)_OBJECTS = $(crypt_OBJECTS)

#========================================================================
# This is a list of header files to be installed
#========================================================================

GENERIC_HDRS= 

#========================================================================
# Add additional lines to handle any additional AC_SUBST cases that
# have been added to the configure script.
#========================================================================

# SAMPLE_NEW_VAR=@SAMPLE_NEW_VAR@

BUILD_LIMIT = 1

# DISTDIR and DEST are used when creating distributions

DISTDIR=/tmp/
DEST=${DISTDIR}/tclhttpd-${VERSION}
TCL_VERSION=8.6
TCLLIB_VERSION=1.4
TCL_DIR=@TCL_DIR@
TCLLIB_DIR=@TCLLIB_DIR@
THREAD_VERSION=2.6

#========================================================================
# Nothing of the variables below this line need to be changed.  Please
# check the TARGETS section below to make sure the make targets are
# correct.
#========================================================================

SHELL = /bin/bash

srcdir = .
top_srcdir = .
prefix = /usr/opt/daq/12.0-pre5/share/tclhttpd
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/tclhttpd3.5.1
pkglibdir = $(libdir)/tclhttpd3.5.1
cryptlibdir = $(libdir)/crypt1.0
limitlibdir = $(libdir)/limit1.0
pkgincludedir = $(includedir)/tclhttpd3.5.1
SERVER_ROOT = $(prefix)/$(PACKAGE)
htdocsdir = $(SERVER_ROOT)/htdocs
customdir = $(SERVER_ROOT)/custom

top_builddir = .

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} -m 755
INSTALL_DATA = ${INSTALL} -m 644 -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM} -m 644
INSTALL_STRIP_FLAG =
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :

PACKAGE = tclhttpd
VERSION = 3.5.1
RELEASE = 1
WIN_VERSION = 351
CC = gcc
CFLAGS_DEBUG = -g
CFLAGS_DEFAULT = -O2
CFLAGS_OPTIMIZE = -O2
CLEANFILES = 
EXEEXT = 
LDFLAGS_DEBUG = @LDFLAGS_DEBUG@
LDFLAGS_DEFAULT = 
LDFLAGS_OPTIMIZE = @LDFLAGS_OPTIMIZE@
MAKE_LIB = ${SHLIB_LD} -o $@ $($@_OBJECTS) ${SHLIB_LDFLAGS} ${SHLIB_LD_LIBS}
MAKE_SHARED_LIB = ${SHLIB_LD} -o $@ $($@_OBJECTS) ${SHLIB_LDFLAGS} ${SHLIB_LD_LIBS}
MAKE_STATIC_LIB = ${STLIB_LD} $@ $($@_OBJECTS)
OBJEXT = o
RANLIB = :
SHLIB_CFLAGS = -fPIC
SHLIB_LD = ${CC} ${CFLAGS} ${LDFLAGS} -shared
SHLIB_LDFLAGS = 
SHLIB_LD_LIBS = -L/usr/lib/x86_64-linux-gnu -ltclstub8.6 -lcrypt
STLIB_LD = ${AR} cr
TCL_BIN_DIR = /usr/lib/tcl8.6
TCL_DEFS = -DPACKAGE_NAME=\"tcl\" -DPACKAGE_TARNAME=\"tcl\" -DPACKAGE_VERSION=\"8.6\" -DPACKAGE_STRING=\"tcl\ 8.6\" -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SYS_PARAM_H=1 -DUSE_THREAD_ALLOC=1 -D_REENTRANT=1 -D_THREAD_SAFE=1 -DHAVE_PTHREAD_ATTR_SETSTACKSIZE=1 -DHAVE_PTHREAD_ATFORK=1 -DTCL_THREADS=1 -DTCL_CFGVAL_ENCODING=\"iso8859-1\" -DHAVE_ZLIB=1 -DMODULE_SCOPE=extern\ __attribute__\(\(__visibility__\(\"hidden\"\)\)\) -DHAVE_HIDDEN=1 -DHAVE_CAST_TO_UNION=1 -DTCL_SHLIB_EXT=\".so\" -DNDEBUG=1 -DTCL_CFG_OPTIMIZED=1 -DTCL_TOMMATH=1 -DMP_PREC=4 -D_LARGEFILE64_SOURCE=1 -DTCL_WIDE_INT_IS_LONG=1 -DHAVE_GETCWD=1 -DHAVE_MKSTEMP=1 -DHAVE_OPENDIR=1 -DHAVE_STRTOL=1 -DHAVE_WAITPID=1 -DHAVE_GETNAMEINFO=1 -DHAVE_GETADDRINFO=1 -DHAVE_FREEADDRINFO=1 -DHAVE_GAI_STRERROR=1 -DHAVE_STRUCT_ADDRINFO=1 -DHAVE_STRUCT_IN6_ADDR=1 -DHAVE_STRUCT_SOCKADDR_IN6=1 -DHAVE_STRUCT_SOCKADDR_STORAGE=1 -DHAVE_GETPWUID_R_5=1 -DHAVE_GETPWUID_R=1 -DHAVE_GETPWNAM_R_5=1 -DHAVE_GETPWNAM_R=1 -DHAVE_GETGRGID_R_5=1 -DHAVE_GETGRGID_R=1 -DHAVE_GETGRNAM_R_5=1 -DHAVE_GETGRNAM_R=1 -DHAVE_GETHOSTBYNAME_R_6=1 -DHAVE_GETHOSTBYNAME_R=1 -DHAVE_GETHOSTBYADDR_R_8=1 -DHAVE_GETHOSTBYADDR_R=1 -DHAVE_TERMIOS_H=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_TIME_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_GMTIME_R=1 -DHAVE_LOCALTIME_R=1 -DHAVE_MKTIME=1 -DHAVE_TM_GMTOFF=1 -DHAVE_TIMEZONE_VAR=1 -DHAVE_STRUCT_STAT_ST_BLOCKS=1 -DHAVE_STRUCT_STAT_ST_BLKSIZE=1 -DHAVE_BLKCNT_T=1 -DHAVE_INTPTR_T=1 -DHAVE_UINTPTR_T=1 -DNO_UNION_WAIT=1 -DHAVE_SIGNED_CHAR=1 -DHAVE_LANGINFO=1 -DHAVE_MKSTEMPS=1 -DHAVE_FTS=1 -DHAVE_SYS_IOCTL_H=1 -DTCL_UNLOAD_DLLS=1 -DHAVE_CPUID=1 
TCL_EXTRA_CFLAGS = -g -O2 -fdebug-prefix-map=/build/tcl8.6-8.6.9+dfsg=. -fstack-protector-strong -Wformat -Werror=format-security -fno-unit-at-a-time -pipe -Wdate-time -D_FORTIFY_SOURCE=2
TCL_LD_FLAGS = -Wl,-z,relro -Wl,--export-dynamic 
TCL_LIBS = -ldl -lz  -lpthread -lm
TCL_SHLIB_LD_LIBS = ${LIBS}
TCL_SRC_DIR = /usr/include/tcl8.6/tcl-private
TCL_DBGX = 
TCL_STUB_LIB_FILE = libtclstub8.6.a
TCL_STUB_LIB_SPEC = -L/usr/lib/x86_64-linux-gnu -ltclstub8.6
TCL_TOOL_DIR_NATIVE = @TCL_TOOL_DIR_NATIVE@
TCL_TOP_DIR_NATIVE = @TCL_TOP_DIR_NATIVE@
TCL_UNIX_DIR_NATIVE = @TCL_UNIX_DIR_NATIVE@
TCL_WIN_DIR_NATIVE = @TCL_WIN_DIR_NATIVE@
INCLUDE_DIR_NATIVE = @INCLUDE_DIR_NATIVE@
TCL_BMAP_DIR_NATIVE = @TCL_BMAP_DIR_NATIVE@
TCL_PLATFORM_DIR_NATIVE = @TCL_PLATFORM_DIR_NATIVE@
TCL_GENERIC_DIR_NATIVE = @TCL_GENERIC_DIR_NATIVE@
TCLSH_PROG = /usr/bin/tclsh8.6
SHARED_BUILD = 1

AUTOCONF = autoconf

LDFLAGS = $(LDFLAGS_DEFAULT)

INCLUDES = -I"/usr/include/tcl8.6"

EXTRA_CFLAGS = $(PROTO_FLAGS) $(SECURITY_FLAGS) $(MEM_DEBUG_FLAGS) $(KEYSYM_FLAGS) $(NO_DEPRECATED_FLAGS)

DEFS =  -DVERSION=\"3.5.1\" -DUSE_TCL_STUBS=1  $(TCL_DEFS) $(TCL_EXTRA_CFLAGS) $(EXTRA_CFLAGS)

ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
CONFIGDIR = ./config
mkinstalldirs = $(SHELL) $(CONFIGDIR)/mkinstalldirs
CONFIG_CLEAN_FILES =

CPPFLAGS = 
LIBS = 
AR = ar
CFLAGS = ${CFLAGS_DEFAULT} ${CFLAGS_WARNING} ${SHLIB_CFLAGS}
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

#========================================================================
# Start of user-definable TARGETS section
#========================================================================

#========================================================================
# TEA TARGETS.  Please note that the "libraries:" target refers to platform
# independent files, and the "binaries:" target inclues executable programs and
# platform-dependent libraries.  Modify these targets so that they install
# the various pieces of your package.  The make and install rules
# for the BINARIES that you specified above have already been done.
#========================================================================

all: binaries libraries doc

#========================================================================
# The binaries target builds executable programs, Windows .dll's, unix
# shared/static libraries, and any other platform-dependent files.
# The list of targets to build for "binaries:" is specified at the top
# of the Makefile, in the "BINARIES" variable.
#========================================================================

binaries: $(BINARIES)


libraries:

doc:

install: all install-binaries install-libraries install-doc

install-binaries: binaries install-lib-binaries

test:

depend:

#========================================================================
# Enumerate the names of the object files included in this package.
# These objects are created and linked into the final library.  In
# most cases these object files will correspond to the source files
# above.
#
# $(exampleA_LIB_FILE) should be listed as part of the BINARIES variable
# at the top of the Makefile.  That will ensure that this target is built
# when you run "make binaries".
#
# You shouldn't need to modify this target, except to change the package
# name from "exampleA" to your package's name.
#========================================================================

# NOTE - MAKE_LIB can't work here, because it wants to reference
# tclhttpd_LIB_OBJECTS, but we need to make two extensions.
# So, we directly jam in the MAKE_SHARED_LIB macro results, but
# only the Unix variant.  These extensions don't apply to Windows anyway

$(crypt_LIB_FILE): $(crypt_OBJECTS)
	-rm -f $(crypt_LIB_FILE)
	${SHLIB_LD} -o $@ $(crypt_OBJECTS) ${SHLIB_LDFLAGS} ${SHLIB_LD_LIBS}

	$(RANLIB) $(crypt_LIB_FILE)

$(limit_LIB_FILE): $(limit_OBJECTS)
	-rm -f $(limit_LIB_FILE)
	${SHLIB_LD} -o $@ $(limit_OBJECTS) ${SHLIB_LDFLAGS} ${SHLIB_LD_LIBS}
	$(RANLIB) $(limit_LIB_FILE)

#========================================================================
# We need to enumerate the list of .c to .o lines here.
# Unfortunately, there does not seem to be any other way to do this
# in a Makefile-independent way.  We can't use VPATH because it picks up
# object files that may be located in the source directory.
#
# In the following lines, $(srcdir) refers to the toplevel directory
# containing your extension.  If your sources are in a subdirectory,
# you will have to modify the paths to reflect this:
#
# exampleA.$(OBJEXT): $(srcdir)/src/win/exampleA.c
# 	$(COMPILE) -c `echo $(srcdir)/src/win/exampleA.c` -o $@
#========================================================================

limit.$(OBJEXT): $(srcdir)/src/limit.c
	$(COMPILE) -c `echo $(srcdir)/src/limit.c` -o $@

crypt.$(OBJEXT): $(srcdir)/src/crypt.c
	$(COMPILE) -c `echo $(srcdir)/src/crypt.c` -o $@

cryptLib.$(OBJEXT): $(srcdir)/src/cryptLib.c
	$(COMPILE) -c `echo $(srcdir)/src/cryptLib.c` -o $@


install-binaries:
	@echo "Installing script programs in $(DESTDIR)$(prefix)/bin"
	@for i in $(srcdir)/bin/*.tcl ; do \
	    $(INSTALL_PROGRAM) $$i $(DESTDIR)$(prefix)/bin ; \
	done;
	@for i in $(srcdir)/bin/*.rc ; do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(prefix)/bin ; \
	done;
	@echo "Installing sample boot script"
	$(INSTALL_DATA) $(srcdir)/bin/tclhttpd.etc.init $(DESTDIR)$(SERVER_ROOT)

install-libraries: install-custom installdirs 
	@echo "Installing script files in $(DESTDIR)$(pkglibdir)"
	@for i in $(srcdir)/lib/*.tcl ; do \
	    $(INSTALL_SCRIPT) $$i $(DESTDIR)$(pkglibdir) ; \
	done;
	$(INSTALL_SCRIPT) $(srcdir)/lib/mime.types $(DESTDIR)$(pkglibdir)

install-htdocs:
	@echo "Creating htdocs tree in $(htdocsdir)"
	-chmod 755 `echo $(srcdir)/htdocs/cgi-bin/*.cgi`
	$(TCLSH_PROG) `echo $(srcdir)/bin/CopyDist` `echo $(srcdir)/htdocs` `echo $(DESTDIR)$(htdocsdir)`
	-$(mkinstalldirs) $(DESTDIR)$(htdocsdir)/links
	@echo "Copying some READMEs into $(htdocsdir)/links"
	$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(htdocsdir)/links/README.txt
	$(INSTALL_DATA) $(srcdir)/certs/README.ssl $(DESTDIR)$(htdocsdir)/links/README.ssl.txt
	$(INSTALL_DATA) $(srcdir)/WHATSNEW $(DESTDIR)$(htdocsdir)/links/WHATSNEW.txt
	$(INSTALL_DATA) $(srcdir)/INSTALL $(DESTDIR)$(htdocsdir)/links/INSTALL.txt
	@echo "Copying license.terms into $(DESTDIR)$(htdocsdir)"
	$(INSTALL_DATA) $(srcdir)/license.terms $(DESTDIR)$(htdocsdir)/license.terms

install-custom:
	@echo "Adding custom code in $(DESTDIR)$(customdir)"
	$(TCLSH_PROG) `echo $(srcdir)/bin/CopyDist` `echo $(srcdir)/custom` `echo $(DESTDIR)$(customdir)`

install-doc: install-htdocs
	@echo "Installing man page"
	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1
	$(INSTALL_DATA) $(srcdir)/doc/httpd.1 $(DESTDIR)$(mandir)/man1/httpd.1
	@echo "Installing README"
	$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(SERVER_ROOT)
	$(INSTALL_DATA) $(srcdir)/bin/README_custom $(DESTDIR)$(SERVER_ROOT)

installdirs:
	$(mkinstalldirs) $(DESTDIR)$(pkglibdir) $(DESTDIR)$(cryptlibdir) $(DESTDIR)$(prefix)/bin $(DESTDIR)$(htdocsdir) $(DESTDIR)$(customdir)

clean:  
	-test -z "$(BINARIES)" || rm -f $(BINARIES)
	-rm -f *.o core *.core
	-rm -f *.$(OBJEXT)
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean: clean
	-rm -f *.tab.c
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*
	-rm -f config.status
	-rm -f limit_pkgIndex.tcl crypt_pkgIndex.tcl

#========================================================================
# Install binary object libraries.  On Windows this includes both .dll and
# .lib files.  Because the .lib files are not explicitly listed anywhere,
# we need to deduce their existence from the .dll file of the same name.
# Additionally, the .dll files go into the bin directory, but the .lib
# files go into the lib directory.  On Unix platforms, all library files
# go into the lib directory.  In addition, this will generate the pkgIndex.tcl
# file in the install location (assuming it can find a usable tclsh8.2 shell)
#
# You should not have to modify this target.
#========================================================================

install-lib-binaries: installdirs
	@list='$(lib_BINARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    ext=`echo $$p|sed -e "s/.*\.//"`; \
	    if test "x$$ext" = "xdll"; then \
	        echo " $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/$$p"; \
	        $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/$$p; \
		lib=`basename $$p|sed -e 's/.[^.]*$$//'`.lib; \
		if test -f $$lib; then \
		    echo " $(INSTALL_PROGRAM) $$lib $(DESTDIR)$(libdir)/$$lib"; \
	            $(INSTALL_PROGRAM) $$lib $(DESTDIR)$(libdir)/$$lib; \
		fi; \
	    else \
		echo " $(INSTALL_PROGRAM) $$p $(DESTDIR)$(libdir)/$$p"; \
	        $(INSTALL_PROGRAM) $$p $(DESTDIR)$(libdir)/$$p; \
	    fi; \
	  else :; fi; \
	done
	@list='$(lib_BINARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(RANLIB) $(DESTDIR)$(libdir)/$$p"; \
	    $(RANLIB) $(DESTDIR)$(libdir)/$$p; \
	  else :; fi; \
	done
	$(INSTALL_SCRIPT) crypt_pkgIndex.tcl $(DESTDIR)$(cryptlibdir)/pkgIndex.tcl
	if test "$(BUILD_LIMIT)" = "1" ; then \
	    $(mkinstalldirs) $(DESTDIR)$(limitlibdir) ; \
	    $(INSTALL_SCRIPT) limit_pkgIndex.tcl $(DESTDIR)$(limitlibdir)/pkgIndex.tcl;\
	fi

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

# dist target

dist:
	rm -rf $(DEST)
	mkdir $(DEST)
	$(TCLSH_PROG) $(srcdir)/bin/CopyDist $(srcdir) $(DEST)
	(cd $(DISTDIR); tar cf - ./tclhttpd-$(VERSION) | gzip > tclhttpd-$(VERSION).tar.gz)
	(cd $(DISTDIR) ; zip -9 -r -o  tclhttpd$(WIN_VERSION).zip  tclhttpd-$(VERSION))

rpm: dist
	rm -rf $(DISTDIR)/dist
	mkdir -p $(DISTDIR)/dist/SPECS
	mkdir -p $(DISTDIR)/dist/SRPMS
	mkdir -p $(DISTDIR)/dist/RPMS
	mkdir -p $(DISTDIR)/dist/SOURCES
	mkdir -p $(DISTDIR)/dist/BUILD
	cp $(DISTDIR)/$(PACKAGE)-$(VERSION).tar.gz $(DISTDIR)/dist/SOURCES/
	cp $(srcdir)/$(PACKAGE).spec $(DISTDIR)/dist/SPECS/$(PACKAGE)-$(VERSION)-$(RELEASE).spec
	rpmbuild --define "_topdir $(DISTDIR)/dist" -ba $(DISTDIR)/dist/SPECS/$(PACKAGE)-$(VERSION)-$(RELEASE).spec

.PHONY: all binaries clean depend distclean doc install installdirs \
libraries test

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
