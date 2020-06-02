// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#define private public
#include <CRingBuffer.h>
#undef private
#include "RingChunk.h"
#include <CRingItem.h>
#include <CRingStateChangeItem.h>

#include <DataFormat.h>
#include <ringbufint.h>     // Internal ring buffer structures.
#include <CAllButPredicate.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <stdlib.h>

extern std::string uniqueName(std::string);

class chunkTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(chunkTest);
  CPPUNIT_TEST(chunk_1);
  CPPUNIT_TEST(chunk_2);
  CPPUNIT_TEST(chunk_3);
  CPPUNIT_TEST(chunk_4);
  CPPUNIT_TEST(chunk_5);
  
  CPPUNIT_TEST(begins_1);
  CPPUNIT_TEST(begins_2);
  CPPUNIT_TEST(begins_3);
  
  CPPUNIT_TEST(ends_1);
  CPPUNIT_TEST(ends_2);
  CPPUNIT_TEST(ends_3);
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
  void chunk_3();
  void chunk_4();
  void chunk_5();
  
  void begins_1();
  void begins_2();
  void begins_3();
  
  void ends_1();
  void ends_2();
  void ends_3();
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
    totalSize += itemSize(pRaw);
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
    pRingItem pItem = reinterpret_cast<pRingItem>(p);
    uint32_t size = itemSize(pItem);
    EQ(uint16_t(PHYSICS_EVENT), itemType(pItem));
    
    pBodyHeader pBh = reinterpret_cast<pBodyHeader>(bodyHeader(pItem));
    EQ(uint64_t(n), pBh->s_timestamp);
    EQ(uint32_t(10-n), pBh->s_sourceId);
    EQ(uint32_t(0), pBh->s_barrier);
    
    size_t bodySize = size - sizeof(RingItemHeader) - sizeof(BodyHeader);
    EQ(size_t(256), bodySize);
    uint8_t* pBody = reinterpret_cast<uint8_t*>(bodyPointer(pItem));
    
    for (int i =0; i < bodySize; i++) {
      EQ(uint8_t(n + i ), pBody[i]);  
    }
    
    p += size;
    nBytes -= size;
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
    totalSize += itemSize(pRaw);
    item.commitToRing(*pRing);
  }
  // Make a ring item header and put it in the ring with a size that's
  // bigger than the header.
  
  RingItemHeader hdr;
  hdr.s_size = sizeof(RingItemHeader) + 100;
  hdr.s_type = PHYSICS_EVENT;
  pRing->put(&hdr, sizeof(RingItemHeader));
  
  // Results should look like the prior one:
  
  ASSERT(!m_pTestObj->nextItemWraps());   // Should not wrap.
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
    pRingItem pItem = reinterpret_cast<pRingItem>(p);
    uint32_t size = itemSize(pItem);
    EQ(uint16_t(PHYSICS_EVENT), itemType(pItem));
    
    pBodyHeader pBh = reinterpret_cast<pBodyHeader>(bodyHeader(pItem));
    EQ(uint64_t(n), pBh->s_timestamp);
    EQ(uint32_t(10-n), pBh->s_sourceId);
    EQ(uint32_t(0), pBh->s_barrier);
    
    size_t bodySize = size - sizeof(RingItemHeader) - sizeof(BodyHeader);
    EQ(size_t(256), bodySize);
    uint8_t* pBody = reinterpret_cast<uint8_t*>(bodyPointer(pItem));
    
    for (int i =0; i < bodySize; i++) {
      EQ(uint8_t(n + i ), pBody[i]);  
    }
    
    p += size;
    nBytes -= size;
    n++;
  }
  EQ(10, n);             // 10 items.
  
  // Remove the chunk from the ring.
    
  pConsumer->skip(data.s_nBytes);
  pConsumer->skip(sizeof(RingItemHeader));    // remove the partial rin gitem.
   
}
void chunkTest::chunk_3()
{
  // We're going to ensure the first ring item overwraps the
  // ring buffer top.. We do this by being pretty dirty -
  // set the put and get pointers 1 byte back from the top
  // and insert our ring items.
  //
  // nextItemWraps should be true.
  // Once we pull that out, we shouild get 9 ring items in the next chunk.
  //
  
  size_t skipBytes = pRing->bytesToTop();
  pRing->skip(skipBytes - 1);
  pConsumer->skip(skipBytes - 1);
  
  // Make the ring items in the ringbuffer and tally the total size:
  // Note the first one should wrap:
  
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
    totalSize += itemSize(pRaw);
    item.commitToRing(*pRing);
  }
  ASSERT(m_pTestObj->nextItemWraps());
  CAllButPredicate pred;
  //  Look at the wrapped item:
  
  CRingItem* pWrappedItem = CRingItem::getFromRing(*pConsumer, pred);
  pRingItem pr =
    reinterpret_cast<pRingItem>(pWrappedItem->getItemPointer());
  EQ(uint16_t(PHYSICS_EVENT), itemType(pr));
  
  pBodyHeader pB = reinterpret_cast<pBodyHeader>(bodyHeader(pr));
  EQ(uint64_t(0), pB->s_timestamp);
  EQ(uint32_t(10), pB->s_sourceId);
  EQ(uint32_t(0),  pB->s_barrier);
  uint8_t* pData = reinterpret_cast<uint8_t*>(pB+1);
  size_t   dataSize = itemSize(pr) - sizeof(RingItemHeader) - sizeof(BodyHeader);
  EQ(size_t(256), dataSize);
  for (int i = 0; i < dataSize; i++) {
    EQ(uint8_t(i), pData[i]);
  }
  
  size_t   nBytes = itemSize(pr);
  int      nItems = 1;
  Chunk    data;
  delete pWrappedItem;
  
  m_pTestObj->getChunk(data);
  EQ(totalSize - nBytes, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(0U, data.s_nEnds);
  
  size_t nRemaining = data.s_nBytes;
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  while (nRemaining) {
    pr = reinterpret_cast<pRingItem>(p);
    EQ(uint16_t(PHYSICS_EVENT), itemType(pr));
    
    pB = reinterpret_cast<pBodyHeader>(bodyHeader(pr));
    EQ(uint64_t(nItems), pB->s_timestamp);
    EQ(uint32_t(10-nItems), pB->s_sourceId);
    EQ(uint32_t(0), pB->s_barrier);
    pData   = reinterpret_cast<uint8_t*>(pB+1);
    dataSize = itemSize(pr) - sizeof(RingItemHeader) - sizeof(BodyHeader);
    EQ(size_t(256), dataSize);
    for (int i =0; i < dataSize; i++) {
      EQ(uint8_t(nItems+i), pData[i]);
    }
    
    nRemaining -= itemSize(pr);
    p          += itemSize(pr);
    nItems++;
  }
  EQ(10, nItems);
}
void chunkTest::chunk_4()
{
  // Same a chunk 3 but we put the entire ring item header prior to
  // the wrap.
  
  size_t skipBytes = pRing->bytesToTop();
  pRing->skip(skipBytes - sizeof(RingItemHeader));
  pConsumer->skip(skipBytes - sizeof(RingItemHeader));
  
  // Make the ring items in the ringbuffer and tally the total size:
  // Note the first one should wrap:
  
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
    totalSize += itemSize(pRaw);
    item.commitToRing(*pRing);
  }
  ASSERT(m_pTestObj->nextItemWraps());
  CAllButPredicate pred;
  //  Look at the wrapped item:
  
  CRingItem* pWrappedItem = CRingItem::getFromRing(*pConsumer, pred);
  pRingItem pr =
    reinterpret_cast<pRingItem>(pWrappedItem->getItemPointer());
  EQ(PHYSICS_EVENT, uint32_t(itemType(pr)));
  
  pBodyHeader pB = reinterpret_cast<pBodyHeader>(bodyHeader(pr));
  EQ(uint64_t(0), pB->s_timestamp);
  EQ(uint32_t(10), pB->s_sourceId);
  EQ(uint32_t(0),  pB->s_barrier);
  uint8_t* pData = reinterpret_cast<uint8_t*>(bodyPointer(pr));
  size_t   dataSize = itemSize(pr) - sizeof(RingItemHeader) - sizeof(BodyHeader);
  EQ(size_t(256), dataSize);
  for (int i = 0; i < dataSize; i++) {
    EQ(uint8_t(i), pData[i]);
  }
  
  size_t   nBytes = itemSize(pr);
  int      nItems = 1;
  Chunk    data;
  delete pWrappedItem;
  
  m_pTestObj->getChunk(data);
  EQ(totalSize - nBytes, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(0U, data.s_nEnds);
  
  size_t nRemaining = data.s_nBytes;
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  while (nRemaining) {
    pr = reinterpret_cast<pRingItem>(p);
    EQ(uint16_t(PHYSICS_EVENT), itemType(pr));
    
    pB = reinterpret_cast<pBodyHeader>(bodyHeader(pr));
    EQ(uint64_t(nItems), pB->s_timestamp);
    EQ(uint32_t(10-nItems), pB->s_sourceId);
    EQ(uint32_t(0), pB->s_barrier);
    pData   = reinterpret_cast<uint8_t*>(pB+1);
    dataSize = itemSize(pr) - sizeof(RingItemHeader) - sizeof(BodyHeader);
    EQ(size_t(256), dataSize);
    for (int i =0; i < dataSize; i++) {
      EQ(uint8_t(nItems+i), pData[i]);
    }
    
    nRemaining -= itemSize(pr);
    p          += itemSize(pr);
    nItems++;
  }
  EQ(10, nItems); 
}
void chunkTest::chunk_5()
{
  // In this test we create two ring items that are exactly split by the
  // top of the ring (e.g. one is at the top, the other at the bottom).
  // 1. nextItemWraps should be false.
  // 2. getChunk should give me the first ring item after which
  // 3. nextItemWraps should be false again.
  // 4. A final getChunk shoulid give me the last ring item.
  //
  
  // Create the ring item and figure out how big it is, set the put/get pointers
  // appropriately and commit the first item.
  
  CRingItem item1(PHYSICS_EVENT, 0x5555555555555555ULL, 0);
  uint8_t* p = static_cast<uint8_t*>(item1.getBodyCursor());
  for (int i =0; i < 256; i++)  {
    *p++ = i;
  }
  item1.setBodyCursor(p);
  item1.updateSize();
  size_t item1Bytes = item1.getItemPointer()->s_header.s_size;
  size_t skipSize   = pRing->bytesToTop() - item1Bytes;
  pRing->skip(skipSize);
  pConsumer->skip(skipSize);
  
  // Ok now the ring item will exactly fit without wrapping:
  
  item1.commitToRing(*pRing);
  
  // A second item.  That should start at the very bottom of the ringbuffer:
  
  CRingItem item2(PHYSICS_EVENT, 0xaaaaaaaaaaaaaaaaULL, 1);
  p = static_cast<uint8_t*>(item2.getBodyCursor());
  for (int i =0; i < 256; i++) {
    *p++ = 2*i;
  }
  item2.setBodyCursor(p);
  item2.updateSize();
  item2.commitToRing(*pRing);
  
  // First item at the top of the ring:
  
  ASSERT(!m_pTestObj->nextItemWraps());
  Chunk data;
  m_pTestObj->getChunk(data);
  EQ(item1Bytes, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(0U, data.s_nEnds);
  
  // Check data for ring item1:
  
  pRingItemHeader ph = static_cast<pRingItemHeader>(data.s_pStart);
  EQ(uint32_t(item1Bytes), ph->s_size);
  EQ(PHYSICS_EVENT, ph->s_type);
  pBodyHeader pbh   = reinterpret_cast<pBodyHeader>(ph+1);
  EQ(uint64_t(0x5555555555555555ULL), pbh->s_timestamp);
  EQ(uint32_t(0), pbh->s_sourceId);
  EQ(uint32_t(0), pbh->s_barrier);
  uint8_t* pb = reinterpret_cast<uint8_t*>(pbh+1);
  for (int i =0; i < 256; i++) {
    EQ(uint8_t(i), pb[i]);
  }
  
  pConsumer->skip(data.s_nBytes);
  
  ASSERT(!m_pTestObj->nextItemWraps());
  m_pTestObj->getChunk(data);
  EQ(item1Bytes, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(0U, data.s_nEnds);
  
  // Check data for ring item 2:
  
  ph = static_cast<pRingItemHeader>(data.s_pStart);
  EQ(uint32_t(item1Bytes), ph->s_size);
  EQ(PHYSICS_EVENT, ph->s_type);
  pbh = reinterpret_cast<pBodyHeader>(ph+1);
  EQ(uint64_t(0xaaaaaaaaaaaaaaaaULL), pbh->s_timestamp);
  EQ(uint32_t(1), pbh->s_sourceId);
  EQ(uint32_t(0), pbh->s_barrier);
  pb = reinterpret_cast<uint8_t*>(pbh+1);
  for (int i =0; i < 256; i++) {
    EQ(uint8_t(2*i), pb[i]);
  }
  
  pConsumer->skip(data.s_nBytes);
  
}

/**
 *  From here on out I think we can assume that wrapped ring items get handled
 *  properly, at least at this level.
 */

void chunkTest::begins_1()
{
  // If I make a chunk consisting of a begin run, getting
  // the chunk will show I have one begin.
  
  m_pTestObj->setRunNumber(1);         // We'll have a begin for run 1.
  
  time_t now = time(nullptr);
  CRingStateChangeItem item(
     uint64_t(1234), 0, 1, BEGIN_RUN, 1, 0, now, std::string("This is the run"));
  item.commitToRing(*pRing);
  
  Chunk data;
  ASSERT(!m_pTestObj->nextItemWraps());
  m_pTestObj->getChunk(data);
  EQ(unsigned(1), data.s_nBegins);
  EQ(unsigned(0), data.s_nEnds);
  
  size_t itemSize = item.getItemPointer()->s_header.s_size;
  EQ(itemSize, data.s_nBytes);
  pRingItem pItem = item.getItemPointer();
  EQ(0, memcmp(pItem, data.s_pStart, itemSize));
}
void chunkTest::begins_2()
{
  // A physics event followed by a begin run get stuffed into the ring.
  // This should be fine and gettable in one chunk.  The begin should get counted.
  // This is not a bizarre case.  Consider multiple dtata sources.  Data source
  // 1 starts and we see its begin and maybe some singles data from it at the start.
  // That might be one chunk. then source 2 starts and we see it's begin in a subsequent
  // chunk along with data from source 1.
  //
  
  m_pTestObj->setRunNumber(1);
  
  CRingItem phys(PHYSICS_EVENT, 0x1234, 1);
  time_t now = time(nullptr);
  CRingStateChangeItem begin(
    uint64_t(1237), 2, 0, BEGIN_RUN, 1, 0, now, std::string("here's a new run")
  );
  phys.commitToRing(*pRing);
  begin.commitToRing(*pRing);
  
  size_t totalSize = phys.getItemPointer()->s_header.s_size +
    begin.getItemPointer()->s_header.s_size;
    
  Chunk data;
  ASSERT(!m_pTestObj->nextItemWraps());
  m_pTestObj->getChunk(data);
  EQ(unsigned(1), data.s_nBegins);
  EQ(unsigned(0), data.s_nEnds);
  EQ(totalSize, data.s_nBytes);
  
  // The first item matches the phys ring item contents:
  
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  size_t item1Size = phys.getItemPointer()->s_header.s_size;
  EQ(0, memcmp(phys.getItemPointer(), p, item1Size));
  
  p += item1Size;             // Pointing to item 2:
  size_t item2Size = begin.getItemPointer()->s_header.s_size;
  EQ(0, memcmp(begin.getItemPointer(), p, item2Size));
  
  pConsumer->skip(totalSize);
  
  
}
void chunkTest::begins_3()
{
  // This test emits a begin run, followed by some physics data
  // followed by another begin run.  The begin run count should be
  // correct and the contents of the chunk should match.
  
  m_pTestObj->setRunNumber(1);
  
  time_t now = time(nullptr);
  CRingStateChangeItem begin1(
    uint64_t(1237), 2, 0, BEGIN_RUN, 1, 0, now, std::string("here's a new run")
  );
  CRingStateChangeItem begin2(
    uint64_t(1238), 1, 0, BEGIN_RUN, 1, 0, now, std::string("here's a new run")
  );
  std::vector<CRingItem*> physicsData;
  size_t totalBytes = begin1.getItemPointer()->s_header.s_size;
  totalBytes += begin2.getItemPointer()->s_header.s_size;
  
  for (int i = 0; i < 100; i++) {
    CRingItem* pItem = new CRingItem(PHYSICS_EVENT, 0x1238+i, 2, 0);
    uint16_t* pBody = static_cast<uint16_t*>(pItem->getBodyCursor());
    size_t nWords = 10  + drand48()*100;
    for (int i = 0; i < nWords; i++) {
      *pBody++ = drand48()*65535;
    }
    pItem->setBodyCursor(pBody);
    pItem->updateSize();
    physicsData.push_back(pItem);
    totalBytes += pItem->getItemPointer()->s_header.s_size;
  }
  
  // Create the run:
  
  begin1.commitToRing(*pRing);
  for (int i =0; i < physicsData.size(); i++) {
    physicsData[i]->commitToRing(*pRing);
  }
  begin2.commitToRing(*pRing);
  
  // If we would have wrapped one of our commits would hang waiting for free
  // space.
  
  ASSERT(!m_pTestObj->nextItemWraps());
  Chunk data;
  m_pTestObj->getChunk(data);
  EQ(totalBytes, data.s_nBytes);
  EQ(2U, data.s_nBegins);
  EQ(0U, data.s_nEnds);
  
  // Contents:
  //     Begin1 is first:
  
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  size_t itemSize;
  
  itemSize = begin1.getItemPointer()->s_header.s_size;
  EQ(0, memcmp(begin1.getItemPointer(), p, itemSize));
  
  p += itemSize;
  
  // Now the 100 physics items:
  
  for (int i =0; i < physicsData.size(); i++) {
    itemSize = physicsData[i]->getItemPointer()->s_header.s_size;
    EQ(0, memcmp(physicsData[i]->getItemPointer(), p, itemSize));
    p += itemSize;
    delete physicsData[i];
  }
  // Now the final begin item.
  
  itemSize = begin2.getItemPointer()->s_header.s_size;
  EQ(0, memcmp(begin2.getItemPointer(), p, itemSize));
  
  
  pConsumer->skip(data.s_nBytes);
}
// Tests for end counting.

void chunkTest::ends_1()
{
  // A lone end run should get counted:
  
  m_pTestObj->setRunNumber(1);
  time_t now = time(nullptr);
  CRingStateChangeItem end(
      0xaaaaaaaaaaaaaaaaUL, 1, 1, END_RUN, 1, 1000, now, std::string("End of the run:")
  );
  end.commitToRing(*pRing);
  
  ASSERT(!m_pTestObj->nextItemWraps());
  Chunk data;
  m_pTestObj->getChunk(data);
  
  size_t itemSize = end.getItemPointer()->s_header.s_size;
  EQ(itemSize, data.s_nBytes);
  EQ(1U, data.s_nEnds);
  EQ(0U, data.s_nBegins);
  
  EQ(0, memcmp(end.getItemPointer(), data.s_pStart, itemSize));
  
  pConsumer->skip(data.s_nBytes);
}

void chunkTest::ends_2()
{
  // First some physics events and then an end run item.

  std::vector<CRingItem*> events;
  size_t totalBytes = 0;
  for (int i =0; i < 100; i++) {
    CRingItem* pItem = new CRingItem(PHYSICS_EVENT, 1000+i, 1);
    int nWords = 10  + 100*drand48();
    uint16_t* pBody = static_cast<uint16_t*>(pItem->getBodyCursor());
    for (int i =0; i < nWords; i++) {
      *pBody++ = 65535*drand48();
    }
    pItem->setBodyCursor(pBody);
    pItem->updateSize();
    totalBytes += pItem->getItemPointer()->s_header.s_size;
    pItem->commitToRing(*pRing);
    events.push_back(pItem);
  }
  
  time_t now = time(nullptr);
  CRingStateChangeItem end(
     0xaaaaaaaaaaaaaaaaUL, 1, 1, END_RUN, 1, 1000, now, std::string("End of the run:")
  );
  end.commitToRing(*pRing);
  totalBytes += end.getItemPointer()->s_header.s_size;
  
  ASSERT(!m_pTestObj->nextItemWraps());
  Chunk data;
  m_pTestObj->getChunk(data);
  EQ(totalBytes, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(1U, data.s_nEnds);

  // Contents of the events:
  
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  
  for (int  i = 0; i < 100; i++) {
    int itemSize = events[i]->getItemPointer()->s_header.s_size;
    EQ(0, memcmp(events[i]->getItemPointer(), p, itemSize));
    
    p += itemSize;
    delete events[i];
  }
  // Contents of the end.
  
  int itemSize = end.getItemPointer()->s_header.s_size;
  EQ(0, memcmp(end.getItemPointer(), p, itemSize));
  pConsumer->skip(data.s_nBytes);  

}
void chunkTest::ends_3()
{
  // An end, soime physics events and a second end.
  
  // This test emits a begin run, followed by some physics data
  // followed by another begin run.  The begin run count should be
  // correct and the contents of the chunk should match.
  
  m_pTestObj->setRunNumber(1);
  
  time_t now = time(nullptr);
  CRingStateChangeItem end1(
    uint64_t(1237), 2, 0, END_RUN, 1, 1248, now, std::string("here's a new run")
  );
  CRingStateChangeItem end2(
    uint64_t(1238), 1, 0, END_RUN, 1, 2000, now, std::string("here's a new run")
  );
  std::vector<CRingItem*> physicsData;
  size_t totalBytes = end1.getItemPointer()->s_header.s_size;
  totalBytes += end2.getItemPointer()->s_header.s_size;
  
  for (int i = 0; i < 100; i++) {
    CRingItem* pItem = new CRingItem(PHYSICS_EVENT, 0x1238+i, 2, 0);
    uint16_t* pBody = static_cast<uint16_t*>(pItem->getBodyCursor());
    size_t nWords = 10  + drand48()*100;
    for (int i = 0; i < nWords; i++) {
      *pBody++ = drand48()*65535;
    }
    pItem->setBodyCursor(pBody);
    pItem->updateSize();
    physicsData.push_back(pItem);
    totalBytes += pItem->getItemPointer()->s_header.s_size;
  }
  
  // Create the run:
  
  end1.commitToRing(*pRing);
  for (int i =0; i < physicsData.size(); i++) {
    physicsData[i]->commitToRing(*pRing);
  }
  end2.commitToRing(*pRing);
  
  // If we would have wrapped one of our commits would hang waiting for free
  // space.
  
  ASSERT(!m_pTestObj->nextItemWraps());
  Chunk data;
  m_pTestObj->getChunk(data);
  EQ(totalBytes, data.s_nBytes);
  EQ(0U, data.s_nBegins);
  EQ(2U, data.s_nEnds);
  
  // Contents:
  //     Begin1 is first:
  
  uint8_t* p = static_cast<uint8_t*>(data.s_pStart);
  size_t itemSize;
  
  itemSize = end1.getItemPointer()->s_header.s_size;
  EQ(0, memcmp(end1.getItemPointer(), p, itemSize));
  
  p += itemSize;
  
  // Now the 100 physics items:
  
  for (int i =0; i < physicsData.size(); i++) {
    itemSize = physicsData[i]->getItemPointer()->s_header.s_size;
    EQ(0, memcmp(physicsData[i]->getItemPointer(), p, itemSize));
    p += itemSize;
    delete physicsData[i];
  }
  // Now the final end item.
  
  itemSize = end2.getItemPointer()->s_header.s_size;
  EQ(0, memcmp(end2.getItemPointer(), p, itemSize));
  
  
  pConsumer->skip(data.s_nBytes);  
}
