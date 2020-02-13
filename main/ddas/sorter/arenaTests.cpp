// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "ReferenceCountedBuffer.h"

#define private public
#include "BufferArena.h"
#undef private


class arenaTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(arenaTest);
  CPPUNIT_TEST(initial_1);
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(arenaTest);


void arenaTest::initial_1()
{
  // We start with an empty buffer pool:
  
  ASSERT(m_pTestObj->m_BufferPool.empty());
}