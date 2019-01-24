// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "swFilterRingDataSource.h"
#include <CDataSource.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRingItemFactory.h>
#include <DataFormat.h>


#include <stdlib.h>
#include "fakedatasource.h"


// mock for CDataSource



class ringsourceTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ringsourceTests);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(emptyBody);
  CPPUNIT_TEST(bodyHeaderEmptyBody);
  CPPUNIT_TEST(beginItem);
  CPPUNIT_TEST_SUITE_END();


private:
  CFakeDataSource*        m_pSrc;
  swFilterRingDataSource* m_pTestObject;
public:
  void setUp() {
    m_pSrc = new CFakeDataSource;
    m_pTestObject = new swFilterRingDataSource(*m_pSrc);
  }
  void tearDown() {
    delete m_pTestObject;
    delete m_pSrc;
    
    m_pTestObject = nullptr;
    m_pSrc        = nullptr;
  }
protected:
  void empty();
  void emptyBody();
  void bodyHeaderEmptyBody();
  void beginItem();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ringsourceTests);


//Empty data source immediately returns 0,nullptr.

void ringsourceTests::empty() {
  std::pair<size_t, void*> item = m_pTestObject->read();
  
  EQ(size_t(0), item.first);
  EQ((void*)(nullptr), item.second);
}
// empty body in the fake data source, returns an empty ring item.
// then end.

void ringsourceTests::emptyBody()
{
  CRingItem* pRingItem = new CRingItem(PHYSICS_EVENT);
  pRingItem->updateSize();
  m_pSrc->addItem(pRingItem);
  
  std::pair<size_t, void*> item = m_pTestObject->read();
  //        header            mbz no body header.
  EQ(sizeof(RingItemHeader) + sizeof(uint32_t), item.first);
  pRingItemHeader pHdr = static_cast<pRingItemHeader>(item.second);
  EQ(PHYSICS_EVENT, pHdr->s_type);
  EQ(uint32_t(item.first), pHdr->s_size);
  
  free(item.second);
  
  item = m_pTestObject->read();
  EQ(size_t(0), item.first);
  EQ((void*)(nullptr), item.second);
}
// Empty ring item but with a body header.

void ringsourceTests::bodyHeaderEmptyBody()
{
  CRingItem* pR1 = new CRingItem(PHYSICS_EVENT, 0x12345, 1234, 0);
  pR1->updateSize();
  m_pSrc->addItem(pR1);
  
  std::pair<size_t, void*> item = m_pTestObject->read();
  EQ(sizeof(RingItemHeader) + sizeof(BodyHeader), item.first);
  pRingItem pRawItem = static_cast<pRingItem>(item.second);
  
  EQ(uint32_t(PHYSICS_EVENT), pRawItem->s_header.s_type);
  EQ(uint32_t(sizeof(BodyHeader)), pRawItem->s_body.u_noBodyHeader.s_mbz);
  
  pBodyHeader ph = &pRawItem->s_body.u_hasBodyHeader.s_bodyHeader;

  EQ(uint64_t(0x12345), ph->s_timestamp);
  EQ(uint32_t(1234), ph->s_sourceId);
  EQ(uint32_t(0),    ph->s_barrier);
  
  
  free(item.second);
}
// Pass a begin run item through:

void ringsourceTests::beginItem()
{
  time_t now = time(nullptr);
  CRingItem* pRItem =
    new CRingStateChangeItem(0x2134, 12, 0, BEGIN_RUN, 11, 0, now, "TestItem");
  pRItem->updateSize();
  m_pSrc->addItem(pRItem);
  
  std::pair<size_t, void*> item = m_pTestObject->read();
  pStateChangeItem pRaw = static_cast<pStateChangeItem>(item.second);
  EQ(
    sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(StateChangeItemBody),
    item.first
  );
  
  EQ(uint32_t(BEGIN_RUN), pRaw->s_header.s_type);
  EQ(item.first, size_t(pRaw->s_header.s_size));
  
  pBodyHeader bh = &(pRaw->s_body.u_hasBodyHeader.s_bodyHeader);
  pStateChangeItemBody b = &(pRaw->s_body.u_hasBodyHeader.s_body);
  
  EQ(uint64_t(0x2134), bh->s_timestamp);
  EQ(uint32_t(12), bh->s_sourceId);
  EQ(uint32_t(0), bh->s_barrier);
  
  EQ(uint32_t(11), b->s_runNumber);
  EQ(uint32_t(0), b->s_timeOffset);
  EQ(uint32_t(now), b->s_Timestamp);
  EQ(std::string("TestItem"), std::string(b->s_title));
  
}