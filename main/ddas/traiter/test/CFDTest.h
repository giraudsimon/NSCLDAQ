

#ifndef CFDTEST_H
#define CFDTEST_H

#include <iostream>
#include <stdint.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TEST
#define TEST
#define private public
#endif 

#include "Trace.hpp"
#include "CFD.hpp"

#include "TraceDefs.h"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif


class CFDTest : public CppUnit::TestFixture
{
    private:
    TraceS null_tr;
    TraceS simple_tr;
    TraceS step_tr;
    TraceS line_tr;

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( CFDTest );

    CPPUNIT_TEST ( testValue );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testValue(); 
};

#endif
