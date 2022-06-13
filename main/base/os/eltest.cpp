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

/** @file:  eltest.cpp
 *  @brief: Tests for CElapsedTime class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <unistd.h>

#include "CElapsedTime.h"

class aTestSuite : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(aTestSuite);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CElapsedTime* m_pTime;
public:
    void setUp() {
        m_pTime = new CElapsedTime;
    }
    void tearDown() {
        delete m_pTime;
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(aTestSuite);

void aTestSuite::test_1()
{
    // sleep a second. the measurement should be longer
    // than one second but < 1.25 to be generous.
    
    usleep(1000000);
    double time = m_pTime->measure();
    ASSERT(time >= 1.0);
    ASSERT(time <= 1.25);
}
