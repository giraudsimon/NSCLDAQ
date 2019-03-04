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


#include <vector>

#include <cppunit/extensions/HelperMacros.h>

#include <CRingItem.h>
#define private public
#define protected public
#include "CRingItemToFragmentTransform.h"
#undef private
#undef protected

#include <array>
#include <vector>
#include <limits>

// A test suite 
class CRingItemToFragmentTransformTest : public CppUnit::TestFixture
{

  private:
    CRingItemToFragmentTransform *m_pTransform;

  public:

    CPPUNIT_TEST_SUITE( CRingItemToFragmentTransformTest );
    CPPUNIT_TEST ( validateIds_0 );
    CPPUNIT_TEST ( validateIds_1 );
    CPPUNIT_TEST ( transform_0 );
    CPPUNIT_TEST ( transform_1 );
    CPPUNIT_TEST ( transform_2 );
    CPPUNIT_TEST ( transform_3 );
    CPPUNIT_TEST ( transform_4 );
    CPPUNIT_TEST ( transform_5 );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void validateIds_0();
    void validateIds_1();
    void transform_0();
    void transform_1();
    void transform_2();
    void transform_3();
    void transform_4();
    void transform_5();

  private:

    void fillBody(CRingItem& item);
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CRingItemToFragmentTransformTest );

void CRingItemToFragmentTransformTest::setUp()
{
  int argc=1;
  const char* argv[] = {"unittests"};
  m_pTransform = new CRingItemToFragmentTransform(0);
  m_pTransform->setAllowedSourceIds({0,1,2});
  m_pTransform->setDefaultSourceId(0);
}

void CRingItemToFragmentTransformTest::tearDown()
{
  delete m_pTransform;
}


// Test that ring item with body header and source id that 
// is unaccepted causes failure
void CRingItemToFragmentTransformTest::validateIds_0 () {
   
  // create ring item with tstamp 0x123456 and source id = 3
  CRingItem item(PHYSICS_EVENT, 0x123456, 3);
  std::array<uint8_t, 8192> buffer;

  CPPUNIT_ASSERT_EQUAL( true, item.hasBodyHeader() );

  CPPUNIT_ASSERT_THROW_MESSAGE(
      "Bad source ids should cause a thrown exception",
      (*m_pTransform)(&item, buffer.data()),
      std::runtime_error);
}


// Test that ring item with body header and accepted source id 
// succeeds
void CRingItemToFragmentTransformTest::validateIds_1 () {
   
  // create ring item with tstamp 0x123456 and source id = 3
  CRingItem item(PHYSICS_EVENT, 0x123456, 0);
  std::array<uint8_t, 8192> buffer;

  CPPUNIT_ASSERT_EQUAL( true, item.hasBodyHeader() );

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Valid source ids should NOT cause a thrown exception",
      (*m_pTransform)(&item, buffer.data()));
}


// Test that a ring item with body header generates a fragment
// with all of the data taken from the body header
void CRingItemToFragmentTransformTest::transform_0() 
{
  uint64_t timestamp = 0x123456;
  uint32_t sourceid = 0;
  CRingItem item(PHYSICS_EVENT, timestamp, sourceid);
  fillBody(item);

  CPPUNIT_ASSERT_EQUAL( true, item.hasBodyHeader() );

  std::array<uint8_t, 8192> buffer;
  ClientEventFragment frag = (*m_pTransform)(&item, buffer.data());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp is correct",
      item.getEventTimestamp(),
      frag.s_timestamp);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id is correct",
      item.getSourceId(),
      frag.s_sourceId);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size is correct",
      item.size(),
      frag.s_size);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier is correct",
      item.getBarrierType(),
      frag.s_barrierType);

  uint8_t* pItem = reinterpret_cast<uint8_t*>(item.getItemPointer());
  CPPUNIT_ASSERT_MESSAGE("Payload is correct",
      std::equal(pItem, pItem+item.size(), reinterpret_cast<uint8_t*>(frag.s_payload)));

}


// Test that failure occurs when the transform is expecting body headers,
// no timestamp extractor is specified,
// and yet data arrives without a body header. 
void CRingItemToFragmentTransformTest::transform_1() 
{
  
  m_pTransform->setExpectBodyHeaders(true);
  CRingItem item(PHYSICS_EVENT);
  fillBody(item);

  CPPUNIT_ASSERT_EQUAL( false, item.hasBodyHeader() );

  std::array<uint8_t, 8192> buffer;
  CPPUNIT_ASSERT_THROW_MESSAGE(
      "Observing an item without body header when expecting them causes throw", 
      ClientEventFragment frag = (*m_pTransform)(&item, buffer.data()),
      std::runtime_error);
}


// Test that a warning is emitted without complete failure when:
// 1) expecting body headers
// 2) timestamp extractor IS DEFINED
// 3) data arrives without body header
void CRingItemToFragmentTransformTest::transform_2() 
{
  m_pTransform->setExpectBodyHeaders(true);
  m_pTransform->setTimestampExtractor([](pPhysicsEventItem pItem) { return uint64_t(10); });

  std::stringstream log;
  auto oldBuf = std::cerr.rdbuf(log.rdbuf());

  CRingItem item(PHYSICS_EVENT);
  fillBody(item);

  CPPUNIT_ASSERT_EQUAL( false, item.hasBodyHeader() );

  std::array<uint8_t, 8192> buffer;
  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Observing an item without body header doesn't throw if timestamp extractor exists", 
      ClientEventFragment frag = (*m_pTransform)(&item, buffer.data())
      );

  CPPUNIT_ASSERT_MESSAGE(
      "Observing an item without body header prints message if timestamp extractor exists", 
      log.str().size() !=  0
      );

  std::cerr.rdbuf(oldBuf);

}

// Test that physics event data without a body header generate a fragment with the information
// specified to the transform object. These should be real timestamps from the extractor
// and also a barrier type of 0.
void CRingItemToFragmentTransformTest::transform_3() 
{
  m_pTransform->setTimestampExtractor([](pPhysicsEventItem pItem) { return uint64_t(10); });
  m_pTransform->setDefaultSourceId(1);

  std::stringstream log;
  auto oldBuf = std::cerr.rdbuf(log.rdbuf());

  CRingItem item(PHYSICS_EVENT);
  fillBody(item);

  CPPUNIT_ASSERT_EQUAL( false, item.hasBodyHeader() );

  std::array<uint8_t, 8192> buffer;
  ClientEventFragment frag = (*m_pTransform)(&item, buffer.data());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp is what is produced by the extractor",
      uint64_t(10),
      frag.s_timestamp);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id is correct",
      uint32_t(1),
      frag.s_sourceId);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size is correct",
      item.size(),
      frag.s_size);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics events should have barrier types of 0",
      uint32_t(0),
      frag.s_barrierType);

  uint8_t* pItem = reinterpret_cast<uint8_t*>(item.getItemPointer());
  CPPUNIT_ASSERT_MESSAGE("Payload is correct",
      std::equal(pItem, pItem+item.size(), reinterpret_cast<uint8_t*>(frag.s_payload)));

  std::cerr.rdbuf(oldBuf);

}

// Test for the treatment of non-physics-event ring items without body headers
// These get assigned UINT64_MAX for a timestamp and also a barrier type matching their
// type if non-scalers.
void CRingItemToFragmentTransformTest::transform_4() 
{
  m_pTransform->setTimestampExtractor([](pPhysicsEventItem pItem) { return uint64_t(10); });
  m_pTransform->setDefaultSourceId(1);

  std::stringstream log;
  auto oldBuf = std::cerr.rdbuf(log.rdbuf());

  CRingItem item(BEGIN_RUN);
  fillBody(item);

  CPPUNIT_ASSERT_EQUAL( false, item.hasBodyHeader() );

  std::array<uint8_t, 8192> buffer;
  ClientEventFragment frag = (*m_pTransform)(&item, buffer.data());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("State change item timestamp is UINT64_MAX",
      std::numeric_limits<uint64_t>::max(),
      frag.s_timestamp);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id is correct",
      uint32_t(1),
      frag.s_sourceId);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size is correct",
      item.size(),
      frag.s_size);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier is correct",
      BEGIN_RUN,
      frag.s_barrierType);

  uint8_t* pItem = reinterpret_cast<uint8_t*>(item.getItemPointer());
  CPPUNIT_ASSERT_MESSAGE("Payload is correct",
      std::equal(pItem, pItem+item.size(), reinterpret_cast<uint8_t*>(frag.s_payload)));

  std::cerr.rdbuf(oldBuf);

}

// Test for the treatment of scaler items without body headers.
// These get assigned UINT64_MAX for a timestamp and also a barrier type matching their
// type if non-scalers.
void CRingItemToFragmentTransformTest::transform_5() 
{
  std::cerr << "transform_5\n";
  m_pTransform->setDefaultSourceId(1);

  std::stringstream log;
  auto oldBuf = std::cerr.rdbuf(log.rdbuf());

  CRingItem item(PERIODIC_SCALERS);
  fillBody(item);

  CPPUNIT_ASSERT_EQUAL( false, item.hasBodyHeader() );

  std::array<uint8_t, 8192> buffer;
  ClientEventFragment frag = (*m_pTransform)(&item, buffer.data());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler item timestamp is UINT64_MAX if not already provided",
      std::numeric_limits<uint64_t>::max(),
      frag.s_timestamp);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id is correct",
      uint32_t(1),
      frag.s_sourceId);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size is correct",
      item.size(),
      frag.s_size);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier is correct",
      uint32_t(0),
      frag.s_barrierType);

  uint8_t* pItem = reinterpret_cast<uint8_t*>(item.getItemPointer());
  CPPUNIT_ASSERT_MESSAGE("Payload is correct",
      std::equal(pItem, pItem+item.size(), reinterpret_cast<uint8_t*>(frag.s_payload)));

  std::cerr.rdbuf(oldBuf);
  std::cerr << " Passed\n";

}

void CRingItemToFragmentTransformTest::fillBody(CRingItem& item)
{

  std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7};
  uint8_t* pData = reinterpret_cast<uint8_t*>(item.getBodyPointer()); 
  pData = std::copy(data.begin(), data.end(), pData);
  item.setBodyCursor(pData);
  item.updateSize();

}

