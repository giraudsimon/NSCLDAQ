

#ifndef TRAPFILTERTEST_H
#define TRAPFILTERTEST_H

#include <iostream>
#include <stdint.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TEST
#define TEST 
#define private public
#endif 

#include "Trace.hpp"
#include "TrapFilter.hpp"
#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class TrapFilterTest : public CppUnit::TestFixture
{

    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS step_tr;
    TraceS line_tr;
    TraceS step_trlong;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( TrapFilterTest );

    CPPUNIT_TEST ( testConstructor );
    CPPUNIT_TEST ( testMinExtent );
    CPPUNIT_TEST ( testMaxExtent );
    CPPUNIT_TEST ( testIncrement );
    CPPUNIT_TEST ( testWhile );
    CPPUNIT_TEST ( testNegative );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testConstructor();

    void testIncrement();

    void testMaxExtent();
    void testMinExtent();

    void testWhile();
    void testNegative();
    
};

#endif
