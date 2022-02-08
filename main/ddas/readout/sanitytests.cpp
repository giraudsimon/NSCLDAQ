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

/** @file:  
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CMyEventSegment.h"

class sanitytest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sanitytest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CMyEventSegment* m_pSeg;
public:
    void setUp() {
        m_pSeg = new CMyEventSegment;
    }
    void tearDown() {
        delete m_pSeg;
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sanitytest);

void sanitytest::test_1()
{
}