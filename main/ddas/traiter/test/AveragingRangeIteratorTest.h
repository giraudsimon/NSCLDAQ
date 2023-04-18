

#ifndef AVERAGINGRANGEITERATORTEST_H
#define AVERAGINGRANGEITERATORTEST_H

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "AveragingRangeIterator.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class AveragingRangeIteratorTest : public CppUnit::TestFixture
{
    private:
    typedef TraceT<int16_t> TraceS; 

    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS step_tr;
    TraceS line_tr;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( AveragingRangeIteratorTest );

    CPPUNIT_TEST ( testConstructor );
    CPPUNIT_TEST ( testIsDoneOnNullTrace );
    CPPUNIT_TEST ( testMoveToInBounds );
    CPPUNIT_TEST ( testMoveToOutOfBounds );
    CPPUNIT_TEST ( testIncrementInBounds );
    CPPUNIT_TEST ( testIncrementOutOfBounds );
    CPPUNIT_TEST ( testSimpleAveragingInRange );
    CPPUNIT_TEST ( testAveragingOutOfRange );
    CPPUNIT_TEST ( testHardBoundary );
    CPPUNIT_TEST ( testFirst );
    CPPUNIT_TEST ( testAveraging );
    CPPUNIT_TEST ( testRangeLongerThanTrace );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testConstructor();
    void testIsDoneOnNullTrace();
    void testMoveToInBounds();
    void testMoveToOutOfBounds();

    void testIncrementInBounds();
    void testIncrementOutOfBounds();

    void testSimpleAveragingInRange();
    void testAveragingOutOfRange();

    void testHardBoundary();

    void testFirst();
    void testAveraging();

    void testRangeLongerThanTrace();
    
};

#endif
