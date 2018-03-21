
bin_PROGRAMS = FileAnalyzer

FileAnalyzer_SOURCES = FileAnalyzer.cpp CSourceCounterFilter.cpp CSourceCounterFilter.h 
FileAnalyzer_CXXFLAGS = -I@top_srcdir@/utilities/filter \
			-I@top_srcdir@/daq/format \
			-I@top_srcdir@/daq/eventbuilder \
			@LIBTCLPLUS_CFLAGS@

FileAnalyzer_LDADD = @top_builddir@/utilities/filter/libfilter.la \
			@top_builddir@/daq/eventbuilder/libFragmentIndex.la \
			 @top_builddir@/daq/format/libdataformat.la \
			 @top_builddir@/base/dataflow/libDataFlow.la

