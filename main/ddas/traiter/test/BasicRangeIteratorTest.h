

#ifndef BASICRANGEITERATORTEST_H
#define BASICRANGEITERATORTEST_H

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TEST
#define TEST
#define private public
#endif

#include "Trace.hpp"

#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class BasicRangeIteratorTest : public CppUnit::TestFixture
{

    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS step_tr;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( BasicRangeIteratorTest );

    CPPUNIT_TEST ( testConstructor );
    CPPUNIT_TEST ( testConstructor2 );
    CPPUNIT_TEST ( testCopyConstructor );
    CPPUNIT_TEST ( testAssignment );
    CPPUNIT_TEST ( testEquality );
    CPPUNIT_TEST ( testIsDoneOnNullTrace );
    CPPUNIT_TEST ( testMoveToInBounds );
    CPPUNIT_TEST ( testMoveToOutOfBounds );
    CPPUNIT_TEST ( testIncrementInBounds );
    CPPUNIT_TEST ( testIncrementOutOfBounds );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testConstructor();
    void testConstructor2();
    void testCopyConstructor();
    void testAssignment();
    void testEquality();
    void testIsDoneOnNullTrace();
    void testMoveToInBounds();
    void testMoveToOutOfBounds();

    void testIncrementInBounds();
    void testIncrementOutOfBounds();

    void testRandomAccess();
    
};

#endif
