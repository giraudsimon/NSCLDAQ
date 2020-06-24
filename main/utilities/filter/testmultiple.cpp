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

/** @file:  testmultiple
 *  @brief: test the test source and sink, using test src and sink and multiple inserts
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CFilterTestSource.h"
#include "CFilterTestSink.h"
#include <CDataFormatItem.h>
#include <CGlomParameters.h>
#include <CRingStateChangeItem.h>
#include <time.h>

class testmultiple : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(testmultiple);
    CPPUNIT_TEST(src_1);
    CPPUNIT_TEST(src_2);
    CPPUNIT_TEST(src_3);
    CPPUNIT_TEST_SUITE_END();

protected:
    void src_1();
    void src_2();
    void src_3();
private:
    CFilterTestSource* m_pSrc;
    CFilterTestSink*   m_pSink;
public:
    void setUp() {
        m_pSrc = new CFilterTestSource;
        m_pSink= new CFilterTestSink;
    }
    void tearDown() {
        delete m_pSrc;
        delete m_pSink;
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(testmultiple);

void testmultiple::src_1()
{
    // Empty data source is an immediate eof:
    
    EQ((CRingItem*)(nullptr), m_pSrc->getItem());
}
void testmultiple::src_2()
{
    // If a put a ring item in the first returned item should
    
    // be equal to the one I took out.  We'll us a ring format item.
    
    CDataFormatItem item;
    m_pSrc->addItem(&item);
    
    CRingItem* pItem = m_pSrc->getItem();
    ASSERT(item == (*pItem));
    
    delete pItem;
    
    EQ((CRingItem*)(nullptr), m_pSrc->getItem());  // No more.
}
void testmultiple::src_3()
{
    // If a put a few items in the src they come out in the right order:
    
    CDataFormatItem item1;
    CGlomParameters item2(100, true, CGlomParameters::first);
    CRingStateChangeItem item3(BEGIN_RUN, 1, 0, time(nullptr), "The title string");
    CRingStateChangeItem item4(END_RUN, 1, 100, time(nullptr), "The title string");
    std::vector<CRingItem*> items = {&item1, &item2, &item3, &item4};
    
    m_pSrc->addItem(&item1);
    m_pSrc->addItem(&item2);
    m_pSrc->addItem(&item3);
    m_pSrc->addItem(&item4);
    
    for (int i =0; i < items.size(); i++ ) {
        CRingItem* p = m_pSrc->getItem();
        ASSERT(p);
        ASSERT(*p == *items[i]);
        
        delete p;
    }
    ASSERT(!m_pSrc->getItem());

}