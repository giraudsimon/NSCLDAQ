lib_LTLIBRARIES = libdaqshm.la

libdaqshm_la_SOURCES = daqshm.cpp os.cpp io.cpp CTimeout.cpp CSemaphore.cpp CPosixBlockingRecordLock.cpp
include_HEADERS      = daqshm.h os.h io.h CTimeout.h CSemaphore.h CPosixBlockingRecordLock.cpp

noinst_HEADERS	     = Asserts.h

libdaqshm_la_LIBADD = @THREADLD_FLAGS@ -lcrypt @LIBEXCEPTION_LDFLAGS@
COMPILATION_FLAGS   = @THREADCXX_FLAGS@ @LIBTCLPLUS_CFLAGS@
libdaqshm_la_CPPFLAGS=$(COMPILATION_FLAGS)


EXTRA_DIST=shm.xml

#----------------------------------------
#
# Tests:

noinst_PROGRAMS    = unittests
unittests_SOURCES = TestRunner.cpp createTests.cpp removeTests.cpp attachTests.cpp \
        detachTests.cpp timeoutTests.cpp semaphoretests.cpp

unittests_CPPFLAGS=$(COMPILATION_FLAGS)

unittests_LDADD   = @builddir@/libdaqshm.la @CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@

TESTS=./unittests
