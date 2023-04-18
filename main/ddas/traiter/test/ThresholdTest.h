
#ifndef BASELINEPROCESSORTEST_H
#define BASELINEPROCESSORTEST_H

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "Threshold.hpp"
#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class ThresholdTest : public CppUnit::TestFixture
{

    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS step_tr;
    TraceS negslope_tr;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( ThresholdTest );

    CPPUNIT_TEST ( testProcess );
    CPPUNIT_TEST ( testFirstValSatisfiesThresh );
    CPPUNIT_TEST ( testNoSatisfaction );
    CPPUNIT_TEST ( testAlgoIteratorProcess );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testConstructor();

    void testProcess();
    void testFirstValSatisfiesThresh(); 
    void testSetOffset();

    void testNoSatisfaction();
    void testAlgoIteratorProcess();
};

#endif
