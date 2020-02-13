// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"



#define private public
#include "ReferenceCountedBuffer.h"
#include "BufferArena.h"
#undef private


class arenaTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(arenaTest);
  CPPUNIT_TEST(initial_1);
  
  CPPUNIT_TEST(alloc_1);
  CPPUNIT_TEST_SUITE_END();


private:
  DDASReadout::BufferArena* m_pTestObj;
public:
  void setUp() {
    m_pTestObj = new DDASReadout::BufferArena;
  }
  void tearDown() {
    delete m_pTestObj;
  }
protected:
  void initial_1();
  void alloc_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(arenaTest);


void arenaTest::initial_1()
{
  // We start with an empty buffer pool:
  
  ASSERT(m_pTestObj->m_BufferPool.empty());
}
void arenaTest::alloc_1()
{
  // Allocation doesn't add to the buffer pool.
  // We get a buffer of exactly the requested size and
  // no reference counts.
  
  DDASReadout::ReferenceCountedBuffer* pBuffer - m_pTestObj->allocate(100);
  EQ(size_t(100), pBuffer->s_size);
  ASSERT(!pBuffer->isReferenced());
  ASSERT(pBuffer->s_pData);
  
  // should just be able to delete it:
  
  CPPUNIT_ASSERT_NO_THROW(
    delete pBuffer;
  );
}