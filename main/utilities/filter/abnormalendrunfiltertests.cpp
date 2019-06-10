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
#include <CRingItem.h>
#include <CFileDataSink.h>

class CException;

#define private public
#define protected public
#include "CAbnormalEndRunFilterHandler.h"
#undef private
#undef protected 

// A test suite 
class CAbnormalEndRunFilterHandlerTest : public CppUnit::TestFixture
{
  private:
    CFilter* m_filter;
    CDataSink* m_sink;

  public:
    CAbnormalEndRunFilterHandlerTest();

    CPPUNIT_TEST_SUITE( CAbnormalEndRunFilterHandlerTest );
    CPPUNIT_TEST ( testGenericItem );
    CPPUNIT_TEST ( testOtherItem );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testGenericItem();
    void testOtherItem();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CAbnormalEndRunFilterHandlerTest );

CAbnormalEndRunFilterHandlerTest::CAbnormalEndRunFilterHandlerTest()
    : m_filter(0)
{}

void CAbnormalEndRunFilterHandlerTest::setUp()
{
  m_sink = new CFileDataSink("test.txt");
  m_filter = new CAbnormalEndRunFilterHandler(*m_sink);
}

void CAbnormalEndRunFilterHandlerTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_filter;
  m_filter=0;

  delete m_sink; 
  m_sink = 0;
  remove(".test.txt");
}

// Create some generic item type to force testing of handleRingItem()
void CAbnormalEndRunFilterHandlerTest::testGenericItem()
{
  CRingItem* item = new CRingItem(1000);    

  CRingItem* new_item;

  // make sure this works fine for all other items
  CPPUNIT_ASSERT_NO_THROW( new_item = m_filter->handleRingItem(item));
  CPPUNIT_ASSERT( item == new_item );
  if (item != new_item) delete new_item;
  delete item;

  // make sure this throws when the item gets sent downstream
  std::unique_ptr<CRingItem> abnEnd(new CRingItem(ABNORMAL_ENDRUN));

  CPPUNIT_ASSERT_THROW( new_item = m_filter->handleRingItem(abnEnd.get()),
                        CException );
}


void CAbnormalEndRunFilterHandlerTest::testOtherItem()
{
  CRingItem* item = new CRingItem(1000);    

  CRingItem* new_item;

  // make sure this works fine for all other items
  CPPUNIT_ASSERT_NO_THROW( new_item = m_filter->handleRingItem(item));
  CPPUNIT_ASSERT( item == new_item );
  if (item != new_item) delete new_item;
  delete item;

  // make sure this throws when the item gets sent downstream
  std::unique_ptr<CRingItem> abnEnd(new CRingItem(ABNORMAL_ENDRUN));

  // this is awful but it is basically what happens in the CCompositeFilter.
  CPPUNIT_ASSERT_THROW( new_item = m_filter->handlePhysicsEventItem(static_cast<CPhysicsEventItem*>(abnEnd.get())),
      CException );
}

