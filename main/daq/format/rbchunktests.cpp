// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CRingBufferChunkAccess.h"
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
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
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