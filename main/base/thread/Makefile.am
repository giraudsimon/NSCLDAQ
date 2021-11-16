lib_LTLIBRARIES = libdaqthreads.la
libdaqthreads_la_SOURCES = dshwrappthreads.cpp  SyncGuard.cpp  Synchronizable.cpp  \
				Thread.cpp	\
				CMutex.cpp	\
				CCondition.cpp	\
				CSynchronizedThread.cpp \
				CGaurdedObject.cpp CBufferQueue.cpp
include_HEADERS = dshwrappthreads.h  dshwrapthreads.h  Runnable.h  SyncGuard.h  \
		Synchronizable.h  Thread.h \
		CMutex.h		\
		CCondition.h CSynchronizedThread.h \
		CGaurdedObject.h CBufferQueue.h CBufferQueue.cpp 

COMPILATION_FLAGS = @PIXIE_CPPFLAGS@ \
	-I@top_srcdir@/base/headers -DUSE_PTHREADS @LIBTCLPLUS_CFLAGS@

libdaqthreads_la_LDFLAGS = -version-info $(SOVERSION) \
		-Wl,"-rpath-link=$(libdir)"	\
		@LIBEXCEPTION_LDFLAGS@		\
		$(THREADLD_FLAGS)
libdaqthreads_la_CXXFLAGS=$(THREADCXX_FLAGS) $(COMPILATION_FLAGS)


EXTRA_DIST=thread.xml
