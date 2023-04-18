
#ifndef PEAKFINDPROCESSORTEST_H
#define PEAKFINDPROCESSORTEST_H

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "PeakFindProcessor.hpp"
#include "Trace.hpp"
#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class PeakFindProcessorTest : public CppUnit::TestFixture
{

    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS simple_tr1;
    TraceS simple_tr2;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( PeakFindProcessorTest );

    CPPUNIT_TEST ( testProcessReturnType );
    CPPUNIT_TEST ( testFindMax );
//    CPPUNIT_TEST ( testFailOffsetOutOfBounds );
    CPPUNIT_TEST ( testSetOffset );
    CPPUNIT_TEST ( testLocalMax );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testProcessReturnType();
    void testFindMax();
//    void testFailOffsetOutOfBounds();
    void testLocalMax();
    void testSetOffset();




};

#endif
