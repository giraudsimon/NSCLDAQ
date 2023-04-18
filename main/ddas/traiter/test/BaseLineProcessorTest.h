
#ifndef BASELINEPROCESSORTEST_H
#define BASELINEPROCESSORTEST_H

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "BaseLineProcessor.hpp"
#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class BaseLineProcessorTest : public CppUnit::TestFixture
{
    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS step_tr;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( BaseLineProcessorTest );

    CPPUNIT_TEST ( testProcessReturnTypeIsBaseLineProcResult );
    CPPUNIT_TEST ( testProcess );
    CPPUNIT_TEST ( testRange );
    CPPUNIT_TEST ( testOffset );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testProcessReturnTypeIsBaseLineProcResult();
    void testProcess();
    void testRange();

    void testOffset();


};

#endif
