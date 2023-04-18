
#ifndef RISETIMEPROCESSORTEST_H
#define RISETIMEPROCESSORTEST_H

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "RiseTimeProcessor.hpp"
#include "Trace.hpp"
#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif

class RiseTimeProcessorTest : public CppUnit::TestFixture
{

    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS negslope_tr;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( RiseTimeProcessorTest );

    CPPUNIT_TEST ( testProcess );
    CPPUNIT_TEST ( testT90AndT10Same );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testProcess();
    void testT90AndT10Same();

};

#endif
