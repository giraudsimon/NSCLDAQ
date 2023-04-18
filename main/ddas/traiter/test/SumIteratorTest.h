

#ifndef AVERAGINGRANGEITERATORTEST_H
#define AVERAGINGRANGEITERATORTEST_H

#include <iostream>
#include <stdint.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TEST
#define TEST
#define private public
#endif 

#include "Trace.hpp"
#include "SumIterator.hpp"
#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class SumIteratorTest : public CppUnit::TestFixture
{
    private:
    typedef SumIterator<uint16_t> SumIterS; 

    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS step_tr;
    TraceS line_tr;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( SumIteratorTest );

    CPPUNIT_TEST ( testConstructor );
    CPPUNIT_TEST ( testIsDoneOnNullTrace );
    CPPUNIT_TEST ( testIncrement );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testConstructor();
    void testIsDoneOnNullTrace();

    void testIncrement();
    
};

#endif
