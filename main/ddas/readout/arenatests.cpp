// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"
#include <memory>
#include <stdexcept>

class ArenaTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ArenaTest);
  CPPUNIT_TEST(alloc_1);
  
  CPPUNIT_TEST(free_1);
  CPPUNIT_TEST(free_2);
  CPPUNIT_TEST(free_3);
  
  CPPUNIT_TEST(recycle);
  CPPUNIT_TEST_SUITE_END();


private:
  DDASReadout::BufferArena* m_pTestObj;
public:
  void setUp() {
    m_pTestObj = new DDASReadout::BufferArena;
  }
  void tearDown() {
    delete m_pTestObj;
    m_pTestObj = nullptr;
  }
protected:
  void alloc_1();
  
  void free_1();
  void free_2();                 // Balanced ref/deref
  void free_3();                 // free referenced buffer throws.
  
  void recycle();               // freed buffers get recycled.
};

CPPUNIT_TEST_SUITE_REGISTRATION(ArenaTest);

void ArenaTest::alloc_1() {
  std::unique_ptr<DDASReadout::ReferenceCountedBuffer> p(m_pTestObj->allocate(100));
  
  ASSERT(!p->isReferenced());
  EQ(size_t(100), p->s_size);
  EQ(size_t(0),   p->s_references);
  ASSERT(p->s_pData);
  
}


void ArenaTest::free_1()
{
  DDASReadout::ReferenceCountedBuffer* p = m_pTestObj->allocate(100);
  
  CPPUNIT_ASSERT_NO_THROW(
    m_pTestObj->free(p)
  );
}

void ArenaTest::free_2()
{
  DDASReadout::ReferenceCountedBuffer* p = m_pTestObj->allocate(100);
  p->reference();
  p->dereference();
  
  CPPUNIT_ASSERT_NO_THROW(
    m_pTestObj->free(p)
  );
}
void ArenaTest::free_3()
{
  DDASReadout::ReferenceCountedBuffer* p = m_pTestObj->allocate(100);
  p->reference();
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->free(p),
    std::logic_error
  );
  
  p->dereference();
  m_pTestObj->free(p);
}
void ArenaTest::recycle()
{
    DDASReadout::ReferenceCountedBuffer* p = m_pTestObj->allocate(100);
    m_pTestObj->free(p);
    
    DDASReadout::ReferenceCountedBuffer* pRe = m_pTestObj->allocate(200);
    
    EQ(p, pRe);                  // Underlying storage will differ though.
    
    EQ(size_t(200), p->s_size);
}