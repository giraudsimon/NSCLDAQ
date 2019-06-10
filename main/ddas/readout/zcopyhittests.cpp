// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "ZeroCopyHit.h"
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"



class ZCopyTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ZCopyTests);
  CPPUNIT_TEST(singleHit);
  CPPUNIT_TEST(multiHit);
  CPPUNIT_TEST(reuse);
  CPPUNIT_TEST_SUITE_END();


private:
  DDASReadout::BufferArena*            m_pArena;
  DDASReadout::ReferenceCountedBuffer* m_pBuffer;
public:
  void setUp() {
    m_pArena = new DDASReadout::BufferArena;
    m_pBuffer = m_pArena->allocate(4096);
  }
  void tearDown() {
     // Tests have to either free the buffer back to the arena or delete it.
     
    m_pBuffer = nullptr;
     
    delete m_pArena;
    m_pArena = nullptr;
  }
protected:
  void singleHit();
  void multiHit();
  void reuse();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ZCopyTests);

void ZCopyTests::singleHit() {
  DDASReadout::ZeroCopyHit hit(100, (uint16_t*)(*m_pBuffer), m_pBuffer, m_pArena);
  
  // Underlying hit data should be the buffer's data:
  
  EQ(m_pBuffer->s_pData, (void*)(hit.s_data));          // Sort of tests the cast.
  hit.freeHit();                               // releases the buffer
  
  DDASReadout::ReferenceCountedBuffer* pBuffer = m_pArena->allocate(100);
  EQ(m_pBuffer, pBuffer);
  
  m_pArena->free(pBuffer);
  
}

void ZCopyTests::multiHit()
{
  uint32_t* pData = (uint32_t*)(*m_pBuffer);
  {
    DDASReadout::ZeroCopyHit hit1(100, pData, m_pBuffer, m_pArena);
    pData += 100;
    DDASReadout::ZeroCopyHit hit2(128, pData, m_pBuffer, m_pArena);
  }                                    // Dereferences the buffer.
  ASSERT(!m_pBuffer->isReferenced());
  
  // Should be in the arena now:
  
  EQ(m_pBuffer, m_pArena->allocate(123));
  m_pArena->free(m_pBuffer);
}

void ZCopyTests::reuse()
{
  uint32_t* pData = (uint32_t*)(*m_pBuffer);
  DDASReadout::ZeroCopyHit hit1(100, pData, m_pBuffer, m_pArena);
  pData += 100;
  DDASReadout::ZeroCopyHit hit2(128, pData, m_pBuffer, m_pArena);
  pData += 128;
  
  // If I re-use hit1 and free hit2 - we still should have a referenced buffer:
  
  hit1.setHit(111, pData, m_pBuffer, m_pArena);
  hit2.freeHit();

  ASSERT(m_pBuffer->isReferenced());
  
  // Getting a buffer should give a new one:
  
  DDASReadout::ReferenceCountedBuffer* pBuffer = m_pArena->allocate(100);
  ASSERT(m_pBuffer != pBuffer);
  hit1.freeHit();                    // Returns to the arena.
  ASSERT(!m_pBuffer->isReferenced());
  delete pBuffer;                     // just kill it off.
  
  
}