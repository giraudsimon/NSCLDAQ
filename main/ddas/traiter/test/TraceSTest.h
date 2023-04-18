
#ifndef BASELINEPROCESSORTEST_H
#define BASELINEPROCESSORTEST_H

#include <iostream>
#include <stdint.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Trace.hpp"

#ifndef USING_TRANAL_NAMESPACE
using namespace TrAnal;
#define USING_TRANAL_NAMESPACE
#endif

class TraceSTest : public CppUnit::TestFixture
{

    private:
    TraceT<uint16_t>* trace;
    std::vector<uint16_t> vec;
    

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( TraceSTest );

    CPPUNIT_TEST ( testArrayAccess );
    CPPUNIT_TEST ( testConstructor1 );
    CPPUNIT_TEST ( testLength );

    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp();
    void tearDown();

    // Begin tests
    void testLength();
    void testArrayAccess();
    void testConstructor1();


};

#endif
