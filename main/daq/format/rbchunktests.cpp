// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#define private public
#include "CRingBufferChunkAccess.h"
#include <CRingBuffer.h>
#include <CRingItem.h>
#undef private

#include <stdint.h>


class rbchunkTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rbchunkTest);
  
  // Iterator tests:
  CPPUNIT_TEST(iconst_1);
  CPPUNIT_TEST(iconst_2);
  CPPUNIT_TEST(iconst_3);
  CPPUNIT_TEST(assignment);
  CPPUNIT_TEST(equals_1);
  CPPUNIT_TEST(equals_2);
  CPPUNIT_TEST(deref_1);
  CPPUNIT_TEST(deref_2);
  CPPUNIT_TEST(preinc_1);
  CPPUNIT_TEST(preinc_2);
  CPPUNIT_TEST(preinc_3);
  CPPUNIT_TEST(postinc_1);
  
  // Chunk tests:
  
  CPPUNIT_TEST(chunkconst);
  CPPUNIT_TEST(setchunk);
  CPPUNIT_TEST(chunksize);
  CPPUNIT_TEST(begin_1);
  CPPUNIT_TEST(begin_2);
  CPPUNIT_TEST(end);
  
  // Chunk access class tests.
  
  CPPUNIT_TEST(caconstruct);
  CPPUNIT_TEST(capoll_1);
  CPPUNIT_TEST(capoll_2);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* m_producer;
  CRingBuffer* m_consumer;
public:
  void setUp() {
    CRingBuffer::create("chunktest");
    m_producer = new CRingBuffer("chunktest", CRingBuffer::producer);
    m_consumer = new CRingBuffer("chunktest", CRingBuffer::consumer);
  }
  void tearDown() {
    
    delete m_producer;
    delete m_consumer;
    m_producer = nullptr;
    m_consumer = nullptr;
    CRingBuffer::remove("chunktest");
  }
protected:
  void iconst_1();
  void iconst_2();
  void iconst_3();
  void assignment();
  void equals_1();
  void equals_2();
  void deref_1();
  void deref_2();
  void preinc_1();
  void preinc_2();
  void preinc_3();
  void postinc_1();
  
  void chunkconst();
  void setchunk();
  void chunksize();
  void begin_1();
  void begin_2();
  void end();
  
  void caconstruct();
  void capoll_1();
  void capoll_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rbchunkTest);

void rbchunkTest::iconst_1() {
  CRingBufferChunkAccess::Chunk::iterator i(nullptr, 0);  // actually end.
  EQ((void*)(nullptr), i.m_pData);
  EQ(size_t(0), i.m_nOffset);
  EQ(size_t(0), i.m_nTotalBytes);
}

void rbchunkTest::iconst_2()
{
  uint8_t buffer[100];
  CRingBufferChunkAccess::Chunk::iterator i(buffer, 100);
  EQ((void*)(buffer), i.m_pData);
  EQ(size_t(0), i.m_nOffset);
  EQ(size_t(100), i.m_nTotalBytes);
}
// Copy construction

void rbchunkTest::iconst_3()
{
  uint8_t buffer[100];
  CRingBufferChunkAccess::Chunk::iterator j(buffer, 100);
  CRingBufferChunkAccess::Chunk::iterator i(j);
  EQ((void*)(buffer), i.m_pData);
  EQ(size_t(0), i.m_nOffset);
  EQ(size_t(100), i.m_nTotalBytes);
  
}

// Assignment:

void rbchunkTest::assignment()
{
  uint8_t buffer[100];
  CRingBufferChunkAccess::Chunk::iterator j(buffer, 100);
  CRingBufferChunkAccess::Chunk::iterator i(nullptr, 0);
   
  i = j;
  
  EQ((void*)(buffer), i.m_pData);
  EQ(size_t(0), i.m_nOffset);
  EQ(size_t(100), i.m_nTotalBytes);
}
// They are equal:

void rbchunkTest::equals_1()
{
  uint8_t buffer[100];
  CRingBufferChunkAccess::Chunk::iterator j(buffer, 100);
  CRingBufferChunkAccess::Chunk::iterator i(j);
 
  ASSERT(j == i);
}
// Not equal

void rbchunkTest::equals_2()
{
  uint8_t buffer[100];
  CRingBufferChunkAccess::Chunk::iterator j(buffer, 100);
  CRingBufferChunkAccess::Chunk::iterator i(nullptr, 0);
  ASSERT(!(i == j));
}

void rbchunkTest::deref_1()
{
  RingItemHeader h = {2*sizeof(uint32_t), PHYSICS_EVENT};
  CRingBufferChunkAccess::Chunk::iterator p(&h, sizeof(h));
  
  EQ(uint32_t(2*sizeof(uint32_t)), p->s_size);
  EQ(PHYSICS_EVENT, p->s_type);
}

void rbchunkTest::deref_2()
{
  RingItemHeader h = {2*sizeof(uint32_t), PHYSICS_EVENT};
  CRingBufferChunkAccess::Chunk::iterator p(&h, sizeof(h));

  RingItemHeader& href(*p);
  EQ(uint32_t(2*sizeof(uint32_t)), href.s_size);
  EQ(PHYSICS_EVENT, href.s_type);
}
void rbchunkTest::preinc_1()
{
  // no change for end:
  
  CRingBufferChunkAccess::Chunk::iterator p(nullptr, 0);  // End.
  CRingBufferChunkAccess::Chunk::iterator& q(++p);
  
  ASSERT(&q == &p);             // Same objects since ++p gives ref.
  EQ((void*)(nullptr), p.m_pData);
  EQ(size_t(0), p.m_nOffset);
  EQ(size_t(0), p.m_nTotalBytes); 
}
void rbchunkTest::preinc_2()
{
  uint32_t buffer[100];
  buffer[0] = 4*sizeof(uint32_t); // Simple ring item.
  buffer[1] = PHYSICS_EVENT;
  buffer[4] = 0x1234;
  
  CRingBufferChunkAccess::Chunk::iterator p(buffer, 100*sizeof(uint32_t));
  ++p;
  EQ(buffer[4], p->s_size);
  
}
void rbchunkTest::preinc_3()            // increment into end.
{
  uint32_t buffer[10];
  buffer[0] = 10*sizeof(uint32_t);
  buffer[1] = PHYSICS_EVENT;
  CRingBufferChunkAccess::Chunk::iterator p(buffer, 10*sizeof(uint32_t));
  ++p;
  // End of iteration.
  
  EQ((void*)(nullptr), p.m_pData);
  EQ(size_t(0), p.m_nOffset);
  EQ(size_t(0), p.m_nTotalBytes);   
}
// we use the fact that the actual increment code is shared and implemented
// in preinc.  Therefore we just need to be sure this is really a pre-inc.
void rbchunkTest::postinc_1()
{
  uint32_t buffer[10];
  buffer[0] = 10*sizeof(uint32_t);
  buffer[1] = PHYSICS_EVENT;
  CRingBufferChunkAccess::Chunk::iterator p(buffer, 10*sizeof(uint32_t));
  EQ(buffer[0], (p++)->s_size);
}

//  Construction gives an empty chunk:

void rbchunkTest::chunkconst()
{
  CRingBufferChunkAccess::Chunk c;
  EQ(size_t(0), c.m_nBytesInChunk);
  EQ((void*)(nullptr), c.m_pStorage);
}
void rbchunkTest::setchunk() {
  uint8_t buffer[100];
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(sizeof(buffer), buffer);
  
  EQ(sizeof(buffer), c.m_nBytesInChunk);
  EQ((void*)(buffer), c.m_pStorage);
}
void rbchunkTest::chunksize()
{
  uint8_t buffer[100];
  CRingBufferChunkAccess::Chunk c;
  
  EQ(size_t(0), c.size());
  
  c.setChunk(sizeof(buffer), buffer);
  EQ(sizeof(buffer), c.size());
  
}
void rbchunkTest::begin_1()
{
  CRingBufferChunkAccess::Chunk c;
  CRingBufferChunkAccess::Chunk::iterator p = c.begin();
  
  // Shoull be end:

  EQ((void*)(nullptr), p.m_pData);
  EQ(size_t(0), p.m_nOffset);
  EQ(size_t(0), p.m_nTotalBytes);   
  
}
void rbchunkTest::begin_2()
{
  RingItemHeader h = {sizeof(RingItemHeader), PHYSICS_EVENT};
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(sizeof(h), &h);
  
  CRingBufferChunkAccess::Chunk::iterator p = c.begin();
  EQ(uint32_t(sizeof(RingItemHeader)), p->s_size);
  EQ(PHYSICS_EVENT, p->s_type);
  
}
void rbchunkTest::end()
{
  RingItemHeader h = {sizeof(RingItemHeader), PHYSICS_EVENT};
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(sizeof(h), &h);

  CRingBufferChunkAccess::Chunk::iterator p = c.end();
  // Shoull be end:

  EQ((void*)(nullptr), p.m_pData);
  EQ(size_t(0), p.m_nOffset);
  EQ(size_t(0), p.m_nTotalBytes);   
  
}
///
void rbchunkTest::caconstruct()
{
  CRingBufferChunkAccess a(m_consumer);
  EQ(m_consumer, a.m_pRingBuffer);
  EQ((void*)(nullptr), a.m_chunk.m_pStorage);
  EQ(size_t(0), a.m_chunk.m_nBytesInChunk);
  EQ((void*)(nullptr), a.m_pWrappedItem);
  EQ(size_t(0), a.m_nWrapSize);
  
}
//
void rbchunkTest::capoll_1()           // no data.
{
  CRingBufferChunkAccess a(m_consumer);
  size_t n = a.waitChunk(100, 1, 0);
  
  EQ(size_t(0), n);
}
void rbchunkTest::capoll_2() // one ring item.
{
  CRingItem item(PHYSICS_EVENT);
  uint32_t* pBody = static_cast<uint32_t*>(item.getBodyCursor());
  *pBody++ = 0x12345678;
  *pBody++ = 0xa5a5a5a5;
  *pBody++ = 0x5a5a5a5a;
  
  item.setBodyCursor(pBody);
  item.updateSize();
  size_t itemSize = item.getItemPointer()->s_header.s_size;
  item.commitToRing(*m_producer);
  
  CRingBufferChunkAccess a(m_consumer);
  size_t n = a.waitChunk(1024, 1, 0);
  EQ(itemSize, n);
}