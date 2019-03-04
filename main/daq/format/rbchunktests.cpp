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