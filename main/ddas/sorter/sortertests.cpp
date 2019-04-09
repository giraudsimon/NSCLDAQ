// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CRingBuffer.h>
#include <CRingBufferChunkAccess.h>
#include <DataFormat.h>
#include <CRingItem.h>
#include <CAllButPredicate.h>

#define private public
#include "DDASSorter.h"
#undef private

#include "ZeroCopyHit.h"
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"


#include <string>
#include <stdint.h>
#include <string.h>

static std::string srcRing="datasource";
static std::string sinkRing("datasink");

class sortertest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sortertest);
  CPPUNIT_TEST(hitout);
  
  CPPUNIT_TEST(ringitemOut);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer*  m_pSourceProducer;         // sorter source ring
  CRingBuffer*  m_pSourceConsumer;
  
  CRingBuffer*  m_pSinkProducer;           // sorter sink ring.
  CRingBuffer*  m_pSinkConsumer;
  
  DDASSorter*   m_pTestObject;
  
  CAllButPredicate all;  
  
public:
  
  void setUp() {
    // In case the rings have dangled from a crash.
    
    try {
      CRingBuffer::remove(srcRing);
    } catch(...) {}
    try {
      CRingBuffer::remove(sinkRing);
    } catch(...) {}
    
    m_pSourceProducer = CRingBuffer::createAndProduce(srcRing);
    m_pSourceConsumer = new CRingBuffer(srcRing, CRingBuffer::consumer);
    
    m_pSinkProducer  = CRingBuffer::createAndProduce(sinkRing);
    m_pSinkConsumer  = new CRingBuffer(sinkRing, CRingBuffer::consumer);
    
    m_pTestObject = new DDASSorter(*m_pSourceConsumer, *m_pSinkProducer);
    
  }
  void tearDown() {
    delete m_pTestObject;
    delete m_pSinkProducer;
    delete m_pSinkConsumer;
    delete m_pSourceProducer;
    delete m_pSourceConsumer;
    
    CRingBuffer::remove(srcRing);
    CRingBuffer::remove(sinkRing);
  }
protected:
  void hitout();
  void ringitemOut();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sortertest);

void sortertest::hitout() {            // Output a hit (zerocopy).
  
  DDASReadout::ZeroCopyHit* pHit = m_pTestObject->allocateHit();
  auto pBuf = m_pTestObject->m_pArena->allocate(128);
  uint8_t* p = (uint8_t*)(*pBuf);
  for (int i =0; i < 128; i++) { *p++ = i; }
  
  pHit->setHit(
    128/sizeof(uint32_t), pBuf->s_pData, pBuf, m_pTestObject->m_pArena
  );
  pHit->s_time = 12345678;
  pHit->s_channelLength = 128/sizeof(uint32_t);
  pHit->s_moduleType = 0xaaaa5555;
  
  m_pTestObject->outputHit(pHit);
  m_pTestObject->freeHit(pHit);
  
  // Sink consumer should be able to pull the ring item out of this.
  
  CRingItem *pItem = CRingItem::getFromRing(*m_pSinkConsumer, all);

  ASSERT(pItem->hasBodyHeader());
  EQ(PHYSICS_EVENT, pItem->type());
  EQ(uint64_t(12345678), pItem->getEventTimestamp());
  EQ(m_pTestObject->m_sid, pItem->getSourceId());
  EQ(uint32_t(0), pItem->getBarrierType());
  EQ(size_t(128) + 2*sizeof(uint32_t), pItem->getBodySize());
  uint32_t* pBody = static_cast<uint32_t*>(pItem->getBodyPointer());
  EQ(uint32_t(size_t(128) + 2*sizeof(uint32_t)), *pBody);
  pBody++;
  EQ(uint32_t(0xaaaa5555), *pBody);
  pBody++;
  uint8_t* p8 = reinterpret_cast<uint8_t*>(pBody);
  for (int i = 0; i < 128; i++) {
    EQ(uint8_t(i), *p8);
    p8++;
  }
  
  delete pItem;  
}

void sortertest::ringitemOut()
{
  CRingItem item(PHYSICS_EVENT, 0x12345678, 12);
  uint32_t* pBody = static_cast<uint32_t*>(item.getBodyCursor());
  for (int i =0; i < 128; i++) {
    *pBody++ = i;
  }
  item.setBodyCursor(pBody);
  
  pRingItem pRawItem = item.getItemPointer();
  m_pTestObject->outputRingItem(reinterpret_cast<pRingItemHeader>(pRawItem));
  
  // Should be able to fetch it back out:
  
  CRingItem* pGotten = CRingItem::getFromRing(*m_pSinkConsumer,all);
  pRingItem pRawGotten = pGotten->getItemPointer();
  
  ASSERT(memcmp(pRawItem, pRawGotten, pRawItem->s_header.s_size) == 0);
  
  delete pGotten;
}