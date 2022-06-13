// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CRingBuffer.h>
#include <CRingBufferChunkAccess.h>
#include <DataFormat.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CAllButPredicate.h>

#define private public
#include "DDASSorter.h"
#undef private

#include "ZeroCopyHit.h"
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"
#include "HitManager.h"

#include <string>
#include <stdint.h>
#include <string.h>
#include <time.h>

static std::string srcRing="datasource";
static std::string sinkRing("datasink");

class sortertest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sortertest);
  CPPUNIT_TEST(hitout);
  
  CPPUNIT_TEST(ringitemOut);
  
  CPPUNIT_TEST(flush);
  
  CPPUNIT_TEST(processhits_1);
  CPPUNIT_TEST(processhits_2);
  
  CPPUNIT_TEST(processchunk_1);
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
  void flush();
  void processhits_1();
  void processhits_2();
  
  void processchunk_1();
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
  EQ((size_t(128) + 2*sizeof(uint32_t)), pItem->getBodySize());
  uint32_t* pBody = static_cast<uint32_t*>(pItem->getBodyPointer());
  EQ(uint32_t((size_t(128) + 2*sizeof(uint32_t))/sizeof(uint16_t)), *pBody);
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
  
  ASSERT(memcmp(pRawItem, pRawGotten, itemSize(pRawItem)) == 0);
  
  delete pGotten;
}

void sortertest::flush()           // Test  hit manager flush -> ringbuffer.
{
  HitManager* pManager = m_pTestObject->m_pHits;
  DDASReadout::BufferArena& arena(*(m_pTestObject->m_pArena));
  
  // Put a few nonesense hits into a buffer, put them in the hit manager
  // then call flushHitManager to push those into the ring buffer.
  
  DDASReadout::ReferenceCountedBuffer& buf(*arena.allocate(1024*sizeof(uint32_t)));
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  uint32_t* pData = (uint32_t*)(buf);
  
  for (int i = 0; i < 10; i++) {
    auto pHit = m_pTestObject->allocateHit();
    pHit->setHit(16, pData, &buf, &arena);
    for (int d = 0; d < 16; d++) {
      *pData++ = d+i;
      
    }
    pHit->s_time =i;
    pHit->s_moduleType = 0xaaaa5555;
    hits.push_back(pHit);
  }
  pManager->addHits(hits);
  
  m_pTestObject->flushHitManager();   // Should result in 10 ring items:
  
  for (uint64_t t = 0; t < 10; t++) {        // t is the timestamp too:
    CRingItem* pItem = CRingItem::getFromRing(*m_pSinkConsumer, all);
    ASSERT(pItem->hasBodyHeader());
    EQ(t, pItem->getEventTimestamp());
    EQ(uint32_t(m_pTestObject->m_sid), pItem->getSourceId());
    EQ(PHYSICS_EVENT, pItem->type());
    
    uint32_t* pBody = static_cast<uint32_t*>(pItem->getBodyPointer());
    EQ(uint32_t((16 +2)*sizeof(uint32_t)/sizeof(uint16_t)), *pBody);
    pBody++;
    EQ(uint32_t(0xaaaa5555), *pBody);
    pBody++;
    for (int i =0; i < 16; i++) {
      EQ(uint32_t(t + i), *pBody);
      pBody++;
    }
    delete pItem;
  }
  
}

// Create a ring item with 100 hits... all in the 10 second window.


static uint32_t* putHit(uint32_t* p, uint64_t timestamp)
{
  *p++ = 4 << 17 | 4 << 12;                  // Lengths.
  *p++ = timestamp & 0xffffffff;
  *p++ = (timestamp >> 32);
  *p++ = 0;                                  // no trace.
  
  return p;
}

void sortertest::processhits_1()
{
  uint32_t moduleType = 0x10100000 | 250;
  CRingItem item(PHYSICS_EVENT, 0, 12, 0, 8192+100);
  uint32_t* pWords = static_cast<uint32_t*>(item.getBodyPointer());
  uint32_t* payload = pWords+1;
  *payload++        = moduleType;   // phone module type + speed (250MHz).
  for(int i = 0; i < 100; i++) {
    payload = putHit(payload, i);         
  }
  *pWords = (payload - pWords) *sizeof(uint32_t) / sizeof(uint16_t);
  item.setBodyCursor(payload);
  item.updateSize();
  
  pRingItemHeader pItem = reinterpret_cast<pRingItemHeader>(item.getItemPointer());
  m_pTestObject->processHits(pItem);
  m_pTestObject->flushHitManager();
  
  // Should be 100 hit ring items in the output ring buffer. We'll check:
  // timestamp.
  // item type.
  // size
  // module type.
  
  double c = DDASReadout::RawChannel::moduleCalibration(moduleType);
  for (int i =0; i < 100; i++) {
    std::unique_ptr<CRingItem> pItem(CRingItem::getFromRing(*m_pSinkConsumer, all));
    
    ASSERT(pItem->hasBodyHeader());
    EQ(uint64_t(i*c) , pItem->getEventTimestamp());
    EQ(PHYSICS_EVENT, pItem->type());
    uint32_t* pSize = static_cast<uint32_t*>(pItem->getBodyPointer());
    EQ(uint32_t(6*sizeof(uint32_t)/sizeof(uint16_t)), *pSize);
    pSize++;
    EQ(moduleType, *pSize);
    
  }
  EQ(size_t(0), m_pSinkConsumer->availableData());
}

void sortertest::processhits_2()    // Ensure appropriate items are emitted.
{
  uint32_t moduleType = 0x10100000 | 250;
  double c = DDASReadout::RawChannel::moduleCalibration(moduleType);
  
  CRingItem item(PHYSICS_EVENT, 0, 12, 0, 8192+100);
  uint32_t* pWords = static_cast<uint32_t*>(item.getBodyPointer());
  uint32_t* payload = pWords+1;
  *payload++        = moduleType;   // phone module type + speed (250MHz).
  for(int i = 0; i < 100; i++) {
    payload = putHit(payload, i*10.0e9/(50*c));         // I think this means half in/half out.
  }
  *pWords = (payload - pWords) *sizeof(uint32_t) / sizeof(uint16_t);
  item.setBodyCursor(payload);
  item.updateSize();
  
  pRingItemHeader pItem = reinterpret_cast<pRingItemHeader>(item.getItemPointer());
  m_pTestObject->processHits(pItem);

  
  // I think there will be 49 items in the ringbuffer:
  
  
  for (int i =0; i < 49; i++) {
    std::unique_ptr<CRingItem> pItem(CRingItem::getFromRing(*m_pSinkConsumer, all));
    
    ASSERT(pItem->hasBodyHeader());
    EQ(uint64_t(i*10.0e9/50) , pItem->getEventTimestamp());
    EQ(PHYSICS_EVENT, pItem->type());
    uint32_t* pSize = static_cast<uint32_t*>(pItem->getBodyPointer());
    EQ(uint32_t(6*sizeof(uint32_t)/sizeof(uint16_t)), *pSize);
    pSize++;
    EQ(moduleType, *pSize);
  }
  
  // Ring buffer should be empty:
  
  EQ(size_t(0), m_pSinkConsumer->availableData());
  
  // Flushing will get the rest of them:
  
  m_pTestObject->flushHitManager();
  
  for (int i =49; i < 100; i++) {
    std::unique_ptr<CRingItem> pItem(CRingItem::getFromRing(*m_pSinkConsumer, all));
    
    ASSERT(pItem->hasBodyHeader());
    EQ(uint64_t(i*10.0e9/50) , pItem->getEventTimestamp());
    EQ(PHYSICS_EVENT, pItem->type());
    uint32_t* pSize = static_cast<uint32_t*>(pItem->getBodyPointer());
    EQ(uint32_t(6*sizeof(uint32_t)/sizeof(uint16_t)), *pSize);
    pSize++;
    EQ(moduleType, *pSize);
  }
  EQ(size_t(0), m_pSinkConsumer->availableData());
}

void sortertest::processchunk_1()           // Chunk has a complete run.
{
  // A run will consist of three ring items:
  //  BEGIN_RUN - should just get passed.
  //  PHYSICS_EVENT - with tight timestamps
  //  END_RUN  - should flush the managed hits and the end run item too.
 
 // Begin run:
  
 CRingStateChangeItem begin(0, 2, 1, BEGIN_RUN, 1, 0, time(nullptr),  "A title");
 begin.commitToRing(*m_pSourceProducer);
 
 // PHYSICS item with raw hits:
 
  uint32_t moduleType = 0x10100000 | 250;
  CRingItem item(PHYSICS_EVENT, 0, 12, 0, 8192+100);
  uint32_t* pWords = static_cast<uint32_t*>(item.getBodyPointer());
  uint32_t* payload = pWords+1;
  *payload++        = moduleType;   // phone module type + speed (250MHz).
  for(int i = 0; i < 100; i++) {
    payload = putHit(payload, i);         
  }
  *pWords = (payload - pWords) *sizeof(uint32_t) / sizeof(uint16_t);
  item.setBodyCursor(payload);
  item.updateSize();
  item.commitToRing(*m_pSourceProducer);
 
 
 // End run.
 
 CRingStateChangeItem end(1234, 2, 2, END_RUN, 1, 0, time(nullptr), "A title");
 end.commitToRing(*m_pSourceProducer);
 
 // Get the chunk from m_pSourceConsumer and process it:
 
  CRingBufferChunkAccess chunkGetter(m_pSourceConsumer);
  auto c = chunkGetter.nextChunk();
  m_pTestObject->processChunk(c);
  
  // The output ring should have the entire run.
  
  CRingItem* pItem = CRingItem::getFromRing(*m_pSinkConsumer, all);
  EQ(BEGIN_RUN, pItem->type());
  ASSERT(pItem->hasBodyHeader());
  EQ(uint64_t(0), pItem->getEventTimestamp());
  EQ(uint32_t(2), pItem->getSourceId());
  EQ(uint32_t(1), pItem->getBarrierType());
  delete pItem;
  
  // The 100 physics items:

 double tsc = DDASReadout::RawChannel::moduleCalibration(moduleType);
 for (int  i = 0; i < 100; i++) {
  pItem = CRingItem::getFromRing(*m_pSinkConsumer, all);
  EQ(PHYSICS_EVENT, pItem->type());
  ASSERT(pItem->hasBodyHeader());
  EQ(uint64_t(i*tsc), pItem->getEventTimestamp());
  EQ(uint32_t(2), pItem->getSourceId());
  delete pItem;
 }
 
 // The end run item
  
  pItem = CRingItem::getFromRing(*m_pSinkConsumer, all);
  EQ(END_RUN, pItem->type());
  ASSERT(pItem->hasBodyHeader());
  EQ(uint64_t(1234), pItem->getEventTimestamp());
  EQ(uint32_t(2),    pItem->getSourceId());
  EQ(uint32_t(2),    pItem->getBarrierType());
  
}