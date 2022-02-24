/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  integrationtests.cpp
 *  @brief: Tests for glom as a whole.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

static const char* argv[] = {
    "./glom", "--dt=100", "--timestamp-policy=first", "--sourceid=10",
    "--maxfragments=100", nullptr
};
static const char* glom="./glom";

class integrationtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(integrationtest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(integrationtest);

void integrationtest::test_1()
{
}