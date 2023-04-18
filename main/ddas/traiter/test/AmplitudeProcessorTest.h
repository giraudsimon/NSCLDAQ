
#ifndef AMPLITUDEPROCESSORTEST_H
#define AMPLITUDEPROCESSORTEST_H

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "AmplitudeProcessor.hpp"
#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class AmplitudeProcessorTest : public CppUnit::TestFixture
{

    private:

    TraceS null_tr;
    TraceS simple_tr;
    TraceS simple_tr1;
    TraceS simple_tr2;
    

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( AmplitudeProcessorTest );

    CPPUNIT_TEST ( testFindAmp );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testDefConstructor();
    void testFindAmp();



};

#endif
