// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#define private public
#include <CRingBuffer.h>
#undef private
#include "RingChunk.h"
#include <CRingItem.h>
#include <DataFormat.h>


extern std::string uniqueName(std::string);

class chunkTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(chunkTest);
  CPPUNIT_TEST(chunk_1);
  CPPUNIT_TEST(chunk_2);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* pRing;
  CRingBuffer* pConsumer;
  CRingChunk*  m_pTestObj;
public:
  void setUp() {
    pRing = CRingBuffer::createAndProduce(uniqueName("evlog"));
    pConsumer = new CRingBuffer(uniqueName("evlog"));   // default tpe is consumer
    m_pTestObj = new CRingChunk(pConsumer);
  }
  void tearDown() {
    delete m_pTestObj;
    delete pRing;
    delete pConsumer;
    CRingBuffer::remove(uniqueName("evlog"));
  }
private: 
protected:
  void chunk_1();
  void chunk_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(chunkTest);

void chunkTest::chunk_1()
{
  // Put a bunch of ring itemss into the hoper.
  // The chunker should find all of them.
  // We'll check contents os well as total size. These will all be
  // physics events.
  
  // Make the ring items in the ringbuffer and tally the total size:
  
  size_t totalSize= 0;
  for (int i = 0; i < 10; i++) {
    CRingItem item(PHYSICS_EVENT, i, 10-i);
    uint8_t* p = static_cast<uint8_t*>(item.getBodyCursor());
    for (int j = 0; j < 256; j++) {
      *p++ = j+i;
    }
    item.setBodyCursor(p);
    item.updateSize();
    pRingItem pRaw = item.getItemPointer();
    totalSize += pRaw->s_header.s_size;
    item.commitToRing(*pRing);
  }
  Chunk data;
  m_pTestObj->getChunk(data);
  
  EQ(totalSize, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(0U, data.s_nEnds);
  
  // Check the data -- should be 10 ring items.
  
  int n = 0;
  size_t nBytes = data.s_nBytes;
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  while (nBytes) {
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(p);
    uint32_t itemSize = pHeader->s_size;
    EQ(PHYSICS_EVENT, pHeader->s_type);
    
    pBodyHeader pBh = reinterpret_cast<pBodyHeader>(pHeader+1);
    EQ(uint64_t(n), pBh->s_timestamp);
    EQ(uint32_t(10-n), pBh->s_sourceId);
    EQ(uint32_t(0), pBh->s_barrier);
    
    size_t bodySize = itemSize - sizeof(RingItemHeader) - sizeof(BodyHeader);
    EQ(size_t(256), bodySize);
    uint8_t* pBody = reinterpret_cast<uint8_t*>(pBh+1);
    
    for (int i =0; i < bodySize; i++) {
      EQ(uint8_t(n + i ), pBody[i]);  
    }
    
    p += itemSize;
    nBytes -= itemSize;
    n++;
  }
  EQ(10, n);             // 10 items.
  
  // Remove the chunk from the ring.
    
  pConsumer->skip(data.s_nBytes);
  
}
void chunkTest::chunk_2()
{
   // This is the same as chunk_1, however we put a partial
   // ring item into the ring  buffer.  The getChunk
   // should not return data from that item.

  // Make the ring items in the ringbuffer and tally the total size:
  
  size_t totalSize= 0;
  for (int i = 0; i < 10; i++) {
    CRingItem item(PHYSICS_EVENT, i, 10-i);
    uint8_t* p = static_cast<uint8_t*>(item.getBodyCursor());
    for (int j = 0; j < 256; j++) {
      *p++ = j+i;
    }
    item.setBodyCursor(p);
    item.updateSize();
    pRingItem pRaw = item.getItemPointer();
    totalSize += pRaw->s_header.s_size;
    item.commitToRing(*pRing);
  }
  // Make a ring item header and put it in the ring with a size that's
  // bigger than the header.
  
  RingItemHeader hdr;
  hdr.s_size = sizeof(RingItemHeader) + 100;
  hdr.s_type = PHYSICS_EVENT;
  pRing->put(&hdr, sizeof(RingItemHeader));
  
  // Results should look like the prior one:
  
  Chunk data;
  m_pTestObj->getChunk(data);
  
  EQ(totalSize, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(0U, data.s_nEnds);
  
  // Check the data -- should be 10 ring items.
  
  int n = 0;
  size_t nBytes = data.s_nBytes;
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  while (nBytes) {
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(p);
    uint32_t itemSize = pHeader->s_size;
    EQ(PHYSICS_EVENT, pHeader->s_type);
    
    pBodyHeader pBh = reinterpret_cast<pBodyHeader>(pHeader+1);
    EQ(uint64_t(n), pBh->s_timestamp);
    EQ(uint32_t(10-n), pBh->s_sourceId);
    EQ(uint32_t(0), pBh->s_barrier);
    
    size_t bodySize = itemSize - sizeof(RingItemHeader) - sizeof(BodyHeader);
    EQ(size_t(256), bodySize);
    uint8_t* pBody = reinterpret_cast<uint8_t*>(pBh+1);
    
    for (int i =0; i < bodySize; i++) {
      EQ(uint8_t(n + i ), pBody[i]);  
    }
    
    p += itemSize;
    nBytes -= itemSize;
    n++;
  }
  EQ(10, n);             // 10 items.
  
  // Remove the chunk from the ring.
    
  pConsumer->skip(data.s_nBytes);
  pConsumer->skip(sizeof(RingItemHeader));    // remove the partial rin gitem.
   
}