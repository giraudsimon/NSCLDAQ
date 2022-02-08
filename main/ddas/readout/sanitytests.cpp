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
#define private public
#include "CMyEventSegment.h"
#undef private
#include <sstream>

class sanitytest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sanitytest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(empty);
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
    void construct_1();
    void empty();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sanitytest);

void sanitytest::construct_1()
{
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
}
// IF I pass an empty block of data there should be no output as well:

void sanitytest::empty()
{
    uint32_t* p = nullptr;
    m_pSeg->m_idToSlots.push_back(2);    // id 0 is slot 2
    
    m_pSeg->checkBuffer(p, 0, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
}