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

#include "CTransparentFilter.h"
#include "CNullFilter.h"

#define private public
#define protected public
#include "CCompositeFilter.h"
#undef private
#undef protected 

// A test suite 
class CCompositeFilterTest : public CppUnit::TestFixture
{
  private:
    // Define a test filter to return some testable results
    class CTestFilter : public CFilter {
      std::string m_history;
      int m_nProcessed;

      public:
      CTestFilter() : m_history(""), m_nProcessed(0) {}
    
      std::string history() { return m_history; }
      int getNProcessed() { return m_nProcessed; }

      virtual CTestFilter* clone() const { return new CTestFilter(*this);}

      virtual CRingItem* handleStateChangeItem(CRingStateChangeItem*) 
      { ++m_nProcessed; return new CRingStateChangeItem(BEGIN_RUN);}

      virtual CRingItem* handleScalerItem(CRingScalerItem* ) 
      { ++m_nProcessed; return new CRingScalerItem(200);}

      virtual CRingItem* handleTextItem(CRingTextItem*) 
      { ++m_nProcessed; 
        std::vector<std::string> str_vec;
        str_vec.push_back("0000");
        str_vec.push_back("1111");
        str_vec.push_back("2222");
        return new CRingTextItem(PACKET_TYPES,str_vec);
      }

      virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* ) 
      { ++m_nProcessed; return new CPhysicsEventItem(4096);}

      virtual CRingItem* 
        handlePhysicsEventCountItem(CRingPhysicsEventCountItem*) 
        { ++m_nProcessed; return new CRingPhysicsEventCountItem(static_cast<uint64_t>(4),
            static_cast<uint32_t>(1001));}

      virtual CRingItem* handleFragmentItem(CRingFragmentItem*)
      { ++m_nProcessed; 
        return new CRingFragmentItem(static_cast<uint64_t>(10101),
            static_cast<uint32_t>(1),
            static_cast<uint32_t>(2),
            reinterpret_cast<void*>(new char[2]),
            static_cast<uint32_t>(3));
      }

      virtual CRingItem* handleRingItem(CRingItem*) 
      { ++m_nProcessed; return new CRingItem(100);}

      virtual void initialize() 
      { m_history += "initialize";}
      virtual void finalize() 
      { m_history += "finalize";}
    };

  private:
    CFilter* m_filter;
    CCompositeFilter* m_compositeTest;
    CCompositeFilter* m_compositeTrans;

  public:
    CCompositeFilterTest();

    CPPUNIT_TEST_SUITE( CCompositeFilterTest );
    CPPUNIT_TEST ( testConstructor );

    CPPUNIT_TEST ( testRegisterFilter );
    CPPUNIT_TEST ( testProcessTransparentFilter );

    CPPUNIT_TEST ( testTransparentStateChangeItem );
    CPPUNIT_TEST ( testTestStateChangeItem );
    CPPUNIT_TEST ( testTransparentScalerItem );
    CPPUNIT_TEST ( testTestScalerItem );
    CPPUNIT_TEST ( testTransparentTextItem );
    CPPUNIT_TEST ( testTestTextItem );
    CPPUNIT_TEST ( testTransparentPhysicsEventItem );
    CPPUNIT_TEST ( testTestPhysicsEventItem );
    CPPUNIT_TEST ( testTransparentPhysicsEventCountItem );
    CPPUNIT_TEST ( testTestPhysicsEventCountItem );
    CPPUNIT_TEST ( testTransparentFragmentItem );
    CPPUNIT_TEST ( testTestFragmentItem );
    CPPUNIT_TEST ( testTransparentGenericItem );
    CPPUNIT_TEST ( testTestGenericItem );
    CPPUNIT_TEST ( testInitialize0 );
    CPPUNIT_TEST ( testFinalize0 );
    CPPUNIT_TEST ( testExitsOnNullReturn );

    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testRegisterFilter();

    void testProcessTransparentFilter();
    void testTransparentStateChangeItem();
    void testTestStateChangeItem();
    void testTestScalerItem();
    void testTransparentScalerItem();
    void testTestTextItem();
    void testTransparentTextItem();
    void testTransparentPhysicsEventItem();
    void testTestPhysicsEventItem();
    void testTransparentPhysicsEventCountItem();
    void testTestPhysicsEventCountItem();
    void testTransparentFragmentItem();
    void testTestFragmentItem();
    void testTransparentGenericItem();
    void testTestGenericItem();
    void testInitialize0();
    void testFinalize0();
    void testExitsOnNullReturn();

//    void testTransparentMainLoop();

  private:
  template<class T>  CRingItem* setupAndRunFilter(CRingItem* item);
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CCompositeFilterTest );

CCompositeFilterTest::CCompositeFilterTest()
    : m_filter(0), m_compositeTest(0), m_compositeTrans(0)
{}

void CCompositeFilterTest::setUp()
{
  m_filter = new CTestFilter;
  m_compositeTest = new CCompositeFilter;
  CTestFilter testFilt;
  m_compositeTest->registerFilter(&testFilt);

  CTransparentFilter transFilt;
  m_compositeTrans = new CCompositeFilter;
  m_compositeTrans->registerFilter(&transFilt);
  
}

void CCompositeFilterTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_compositeTrans;
  m_compositeTrans=0;

  delete m_compositeTest;
  m_compositeTest=0;

  delete m_filter;
  m_filter=0;
}


void CCompositeFilterTest::testConstructor()
{
  CCompositeFilter filter;
  CPPUNIT_ASSERT(filter.begin() == filter.end()); 
}



void CCompositeFilterTest::testRegisterFilter()
{
  CCompositeFilter filter;
  CPPUNIT_ASSERT(filter.begin() == filter.end()); 
  filter.registerFilter(m_filter);

  // Check that this is no longer empty
  CPPUNIT_ASSERT(filter.begin() != filter.end());
  // It should have only one filter
  CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1) , filter.size() );
  
}

void CCompositeFilterTest::testProcessTransparentFilter()
{
  // CReate a generic new item to pass in
  CRingItem* item = new CRingItem(100,100); 
  // Setup composite with transparent filter and handle the 
  // ring item with it 
  CRingItem* new_item = m_compositeTrans->handleRingItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );
  
  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

void CCompositeFilterTest::testTransparentStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingStateChangeItem* item = new CRingStateChangeItem(END_RUN); 
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_compositeTrans->handleStateChangeItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
} 

void CCompositeFilterTest::testTestStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingStateChangeItem* item = new CRingStateChangeItem(END_RUN); 
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_compositeTest->handleStateChangeItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item != item );

  // Test filter should always return type BEGIN_RUN
  CPPUNIT_ASSERT( BEGIN_RUN == new_item->type() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;

} 

// Create scaler item
void CCompositeFilterTest::testTransparentScalerItem()
{
  CRingScalerItem* item = new CRingScalerItem (300);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_compositeTrans->handleScalerItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create scaler item
void CCompositeFilterTest::testTestScalerItem()
{
  CRingScalerItem* item = new CRingScalerItem (300);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_compositeTest->handleScalerItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item != item );

  CRingScalerItem* new_sclr = static_cast<CRingScalerItem*>(new_item);
  CPPUNIT_ASSERT( 200 == new_sclr->getScalerCount() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}
  
// Text item
void CCompositeFilterTest::testTransparentTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem* item = new CRingTextItem(MONITORED_VARIABLES,str_vec);

  CRingItem* new_item = m_compositeTrans->handleTextItem(item);

  CPPUNIT_ASSERT (new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;

}

// Text item
void CCompositeFilterTest::testTestTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem* item = new CRingTextItem(MONITORED_VARIABLES,str_vec);
  CRingItem* new_item = m_compositeTest->handleTextItem(item);
  // Test filter should always return 200 scalers
  CRingTextItem* new_text = dynamic_cast<CRingTextItem*>(new_item);

  CPPUNIT_ASSERT( 3 == new_text->getStrings().size() );
  CPPUNIT_ASSERT( "0000" == new_text->getStrings()[0] );
  CPPUNIT_ASSERT( "1111" == new_text->getStrings()[1] );
  CPPUNIT_ASSERT( "2222" == new_text->getStrings()[2] );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// PhysicsEvent item
void CCompositeFilterTest::testTransparentPhysicsEventItem()
{
  CPhysicsEventItem* item = new CPhysicsEventItem(8192);

  CRingItem* new_item = m_compositeTrans->handlePhysicsEventItem(item);

  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEvent item
void CCompositeFilterTest::testTestPhysicsEventItem()
{
  CPhysicsEventItem* item = new CPhysicsEventItem(8192);

  CRingItem* new_item = m_compositeTest->handlePhysicsEventItem(item);
  CPPUNIT_ASSERT( item != new_item );

  CPhysicsEventItem* new_evt = dynamic_cast<CPhysicsEventItem*>(new_item);

  CPPUNIT_ASSERT( 4096 == new_evt->getStorageSize() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEventCount item
void CCompositeFilterTest::testTransparentPhysicsEventCountItem()
{
  CRingPhysicsEventCountItem* item = 0;
  item =  new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  CRingItem* new_item = m_compositeTrans->handlePhysicsEventCountItem(item);
  CRingPhysicsEventCountItem* new_cnt 
    = dynamic_cast<CRingPhysicsEventCountItem*>(new_item);

  CPPUNIT_ASSERT( item == new_item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEventCount item
void CCompositeFilterTest::testTestPhysicsEventCountItem()
{
  CRingPhysicsEventCountItem* item = 0;
  item =  new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  CRingItem* new_item = m_compositeTest->handlePhysicsEventCountItem(item);
  CRingPhysicsEventCountItem* new_cnt 
    = dynamic_cast<CRingPhysicsEventCountItem*>(new_item);

  CPPUNIT_ASSERT( item != new_item);
  CPPUNIT_ASSERT( static_cast<uint64_t>(4) == new_cnt->getEventCount() );
  CPPUNIT_ASSERT( static_cast<uint32_t>(1001) == new_cnt->getTimeOffset() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// RingFragmentItem
void CCompositeFilterTest::testTransparentFragmentItem()
{
  CRingFragmentItem* item = new CRingFragmentItem(static_cast<uint64_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        reinterpret_cast<void*>(0),
                                                        static_cast<uint32_t>(0));

  CRingItem* new_item = m_compositeTrans->handleFragmentItem(item);

  CPPUNIT_ASSERT( new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// RingFragmentItem
void CCompositeFilterTest::testTestFragmentItem()
{
  CRingFragmentItem* item = new CRingFragmentItem(static_cast<uint64_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        reinterpret_cast<void*>(0),
                                                        static_cast<uint32_t>(0));

  CRingItem* new_item = m_compositeTest->handleFragmentItem(item);

  CPPUNIT_ASSERT( new_item != item);

  CRingFragmentItem* new_frag = dynamic_cast<CRingFragmentItem*>(new_item);
  CPPUNIT_ASSERT( static_cast<uint64_t>(10101) == new_frag->timestamp());
  CPPUNIT_ASSERT( static_cast<uint32_t>(1) == new_frag->source());
  CPPUNIT_ASSERT( static_cast<uint32_t>(2) == const_cast<CRingFragmentItem*>(new_frag)->payloadSize());
  CPPUNIT_ASSERT( static_cast<uint32_t>(3) == new_frag->barrierType());

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTransparentGenericItem()
{
  CRingItem* item = new CRingItem(1000);    
  CRingItem* new_item = m_compositeTrans->handleRingItem(item);
  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTestGenericItem()
{
  CRingItem* item = new CRingItem(1000);    
  CRingItem* new_item = m_compositeTest->handleRingItem(item);
  CPPUNIT_ASSERT( item != new_item );
  // Test filter should always return type 100
  CPPUNIT_ASSERT( 100 == new_item->type() );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}


void CCompositeFilterTest::testInitialize0()
{
  CTestFilter f;
  m_compositeTest->registerFilter(&f); 
  m_compositeTest->registerFilter(&f); 

  m_compositeTest->initialize();
  CCompositeFilter::iterator it = m_compositeTest->begin();
  CCompositeFilter::iterator itend = m_compositeTest->end();
  while (it!=itend) {
    CTestFilter *tfilt = static_cast<CTestFilter*>(*it);
    CPPUNIT_ASSERT_EQUAL(std::string("initialize"), tfilt->history());
    ++it;
  }

}

void CCompositeFilterTest::testFinalize0()
{
  CTestFilter f;
  m_compositeTest->registerFilter(&f); 
  m_compositeTest->registerFilter(&f); 

  m_compositeTest->finalize();
  CCompositeFilter::iterator it = m_compositeTest->begin();
  CCompositeFilter::iterator itend = m_compositeTest->end();
  while (it!=itend) {
    CTestFilter *tfilt = static_cast<CTestFilter*>(*it);
    CPPUNIT_ASSERT_EQUAL(std::string("finalize"), tfilt->history());
    ++it;
  }

}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testExitsOnNullReturn()
{
  CRingItem *item = new CRingItem(100);
  // Register it
  CCompositeFilter composite;

  // Create a test filter 
  CFilter* m_filter = new CNullFilter;
  composite.registerFilter(m_filter);

  m_filter = new CTestFilter;
  composite.registerFilter(m_filter);

  CRingItem* new_item = composite.handleRingItem(item);

  // Check that it does return 0
  CPPUNIT_ASSERT( 0 == new_item );

  // check that the test filter never got called
  CCompositeFilter::iterator it = composite.begin();
  ++it;
  CTestFilter* theTest = static_cast<CTestFilter*>(*it);

  CPPUNIT_ASSERT( 0 == theTest->getNProcessed() );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}

