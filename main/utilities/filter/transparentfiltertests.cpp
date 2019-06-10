/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";


#include <cppunit/extensions/HelperMacros.h>
#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <fstream>
#include <vector>
#include <CPhysicsEventItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>

#define private public
#define protected public
#include "CTransparentFilter.h"
#undef private
#undef protected 

// A test suite 
class CTransparentFilterTest : public CppUnit::TestFixture
{
  private:
    CFilter* m_filter;

  public:
    CTransparentFilterTest();

    CPPUNIT_TEST_SUITE( CTransparentFilterTest );

    CPPUNIT_TEST ( testStateChangeItem );
    CPPUNIT_TEST ( testScalerItem );
    CPPUNIT_TEST ( testTextItem );
    CPPUNIT_TEST ( testPhysicsEventItem );
    CPPUNIT_TEST ( testPhysicsEventCountItem );
    CPPUNIT_TEST ( testFragmentItem );
    CPPUNIT_TEST ( testGenericItem );

    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testStateChangeItem();
    void testScalerItem();
    void testTextItem();
    void testPhysicsEventItem();
    void testPhysicsEventCountItem();
    void testFragmentItem();
    void testGenericItem();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CTransparentFilterTest );

CTransparentFilterTest::CTransparentFilterTest()
    : m_filter(0)
{}

void CTransparentFilterTest::setUp()
{
  m_filter = new CTransparentFilter;
}

void CTransparentFilterTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_filter;
  m_filter=0;
}

// Create scaler item
void CTransparentFilterTest::testStateChangeItem()
{
  CRingStateChangeItem* item = new CRingStateChangeItem (BEGIN_RUN);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_filter->handleStateChangeItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create scaler item
void CTransparentFilterTest::testScalerItem()
{
  CRingScalerItem* item = new CRingScalerItem (300);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_filter->handleScalerItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Text item
void CTransparentFilterTest::testTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem* item = new CRingTextItem(MONITORED_VARIABLES,str_vec);

  CRingItem* new_item = m_filter->handleTextItem(item); 

  CPPUNIT_ASSERT (new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;

}

// PhysicsEvent item
void CTransparentFilterTest::testPhysicsEventItem()
{
  CPhysicsEventItem* item = new CPhysicsEventItem(8192);

  CRingItem* new_item = m_filter->handlePhysicsEventItem(item);

  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEventCount item
void CTransparentFilterTest::testPhysicsEventCountItem()
{
  CRingPhysicsEventCountItem* item = 0;
  item =  new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  CRingItem* new_item = m_filter->handlePhysicsEventCountItem(item); 

  CPPUNIT_ASSERT( item == new_item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// RingFragmentItem
void CTransparentFilterTest::testFragmentItem()
{
  CRingFragmentItem* item = new CRingFragmentItem(static_cast<uint64_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        reinterpret_cast<void*>(0),
                                                        static_cast<uint32_t>(0));

  CRingItem* new_item = m_filter->handleFragmentItem(item);
  CPPUNIT_ASSERT( new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create some generic item type to force testing of handleRingItem()
void CTransparentFilterTest::testGenericItem()
{
  CRingItem* item = new CRingItem(1000);    
  CRingItem* new_item = m_filter->handleRingItem(item); 
  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}

