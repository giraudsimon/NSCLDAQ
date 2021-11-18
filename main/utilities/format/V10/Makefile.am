lib_LTLIBRARIES = libdataformatv10.la

libdataformatv10_la_SOURCES = CRingItem.cpp \
                           CRingScalerItem.cpp \
                           CRingTimestampedRunningScalerItem.cpp \
                           CRingStateChangeItem.cpp \
                           CPhysicsEventItem.cpp \
                           CRingPhysicsEventCountItem.cpp \
                           CRingTextItem.cpp \
                           CRingFragmentItem.cpp \
                           CUnknownFragment.cpp \
                           CRingItemFactory.cpp \
                           ringitem.cpp

nscldaq10dir = @includedir@/V10

nscldaq10_HEADERS = CRingItem.h \
                  CRingScalerItem.h \
                  CRingTimestampedRunningScalerItem.h \
                  CRingStateChangeItem.h \
                  CPhysicsEventItem.h \
                  CRingPhysicsEventCountItem.h \
                  CRingTextItem.h \
                  CRingFragmentItem.h \
                  CUnknownFragment.h \
                  CRingItemFactory.h \
                  CVoidItem.h \
                  DataFormatV10.h

libdataformatv10_la_CXXFLAGS = -I@srcdir@/.. \
                               -I@top_srcdir@/utilities/Buffer \
                               @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@

libdataformatv10_la_LDFLAGS = @top_builddir@/utilities/Buffer/libbuffer.la \
                              @LIBTCLPLUS_LDFLAGS@

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp \
                                                                                ringitemtests.cpp teststate.cpp	\
                                                                                scalertests.cpp texttest.cpp physcounttests.cpp		\
                                                                                formatoutputtests.cpp physeventtests.cpp

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
                        @top_builddir@/utilities/Buffer/libbuffer.la \
			@builddir@/libdataformatv10.la	\
                        @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@srcdir@/.. \
		    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/utilities/Buffer \
                    -I@top_srcdir@/base/testutils @PIXIE_CPPFLAGS@

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
