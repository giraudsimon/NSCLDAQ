
#ifndef CMOCKREADOUTLISTTESTS_CPP
#define CMOCKREADOUTLISTTESTS_CPP

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <iostream>

#include <CLoggingReadoutList.h>

using namespace std;

class CLoggingReadoutListTests : public CppUnit::TestFixture
{
  private:
    CLoggingReadoutList*  m_pList;

  CPPUNIT_TEST_SUITE(CLoggingReadoutListTests);
  CPPUNIT_TEST(clear_0);
  CPPUNIT_TEST(append_0);
  CPPUNIT_TEST(addRegisterRead_0);
  CPPUNIT_TEST(addWrite32_0);
  CPPUNIT_TEST(addWrite16_0);
  CPPUNIT_TEST(addWrite8_0);
  CPPUNIT_TEST(addRead32_0);
  CPPUNIT_TEST(addRead16_0);
  CPPUNIT_TEST(addRead8_0);
  CPPUNIT_TEST(addBlockRead32_0);
  CPPUNIT_TEST(addFifoRead32_0);
  CPPUNIT_TEST(addFifoRead16_0);
  CPPUNIT_TEST(addBlockWrite32_0);
  CPPUNIT_TEST(addBlockCountRead8_0);
  CPPUNIT_TEST(addBlockCountRead16_0);
  CPPUNIT_TEST(addBlockCountRead32_0);
  CPPUNIT_TEST(addMaskedCountBlockRead32_0);
  CPPUNIT_TEST(addMaskedCountFifoRead32_0);
  CPPUNIT_TEST(addDelay_0);
  CPPUNIT_TEST(addMarker_0);
  CPPUNIT_TEST_SUITE_END();

  public:
    void setUp() {
      m_pList = new CLoggingReadoutList;
    }

    void tearDown() { 
      delete m_pList;
    }

    void clear_0();
    void append_0();
    void addRegisterRead_0();
    void addWrite32_0();
    void addWrite16_0();
    void addWrite8_0();

    void addRead32_0();
    void addRead16_0();
    void addRead8_0();
    void addBlockRead32_0();
    void addFifoRead32_0();
    void addFifoRead16_0();
    void addBlockWrite32_0();
    void addBlockCountRead8_0();
    void addBlockCountRead16_0();
    void addBlockCountRead32_0();
    void addMaskedCountBlockRead32_0();
    void addMaskedCountFifoRead32_0();
    void addDelay_0();
    void addMarker_0();

};



CPPUNIT_TEST_SUITE_REGISTRATION(CLoggingReadoutListTests);

template<class T>
void print_vectors(const std::vector<T>& expected, const std::vector<T>& actual) {
  std::cout.flags(std::ios::hex);

  std::copy(expected.begin(), expected.end(), std::ostream_iterator<T>(std::cout,"\n"));
  std::cout << "---" << std::endl;
  std::copy(actual.begin(), actual.end(), std::ostream_iterator<T>(std::cout,"\n"));

  std::cout.flags(std::ios::dec);
}

void CLoggingReadoutListTests::clear_0()
{
  m_pList->addDelay(1);
  CPPUNIT_ASSERT( m_pList->size() > size_t(0));
  CPPUNIT_ASSERT( m_pList->logSize() > size_t(0));

  m_pList->clear();

  CPPUNIT_ASSERT( size_t(m_pList->size()) == size_t(0));
  CPPUNIT_ASSERT(m_pList->logSize() == size_t(0));
}

void CLoggingReadoutListTests::append_0()
{
  CLoggingReadoutList list;

  // we only care that we stick something on it
  list.addDelay(20);

  // check that the value is non-zero
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), list.size());

  // make sure that the original is empty
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), m_pList->size());
  
  m_pList->append(list);
  // Check that the list now has the 1 element
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_pList->size());
}

void CLoggingReadoutListTests::addRegisterRead_0()
{
  // we only care that we stick something on it
  m_pList->addRegisterRead(4);

  vector<string> expected(1);
  expected[0] = "addRegisterRead 00000004";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addWrite32_0()
{
  // we only care that we stick something on it
  m_pList->addWrite32(0xabcdef, 0x09, 255);

  vector<string> expected(1);
  expected[0] = "addWrite32 00abcdef 09 255";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addWrite16_0()
{
  // we only care that we stick something on it
  m_pList->addWrite16(0xabcdef, 0x09, 255);

  vector<string> expected(1);
  expected[0] = "addWrite16 00abcdef 09 255";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addWrite8_0()
{
  // we only care that we stick something on it
  m_pList->addWrite8(0xabcdef, 0x09, 255);

  vector<string> expected(1);
  expected[0] = "addWrite8 00abcdef 09 255";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addRead32_0()
{
  // we only care that we stick something on it
  m_pList->addRead32(0xabcdef, 0x09);

  vector<string> expected(1);
  expected[0] = "addRead32 00abcdef 09";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addRead16_0()
{
  // we only care that we stick something on it
  m_pList->addRead16(0xabcdef, 0x09);

  vector<string> expected(1);
  expected[0] = "addRead16 00abcdef 09";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addRead8_0()
{
  // we only care that we stick something on it
  m_pList->addRead8(0xabcdef, 0x09);

  vector<string> expected(1);
  expected[0] = "addRead8 00abcdef 09";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addBlockRead32_0()
{
  // we only care that we stick something on it
  m_pList->addBlockRead32(0xabcdef, 0x09, 240);

  vector<string> expected(1);
  expected[0] = "addBlockRead32 00abcdef 09 240";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addFifoRead32_0()
{
  // we only care that we stick something on it
  m_pList->addFifoRead32(0xabcdef, 0x09, 240);

  vector<string> expected(1);
  expected[0] = "addFifoRead32 00abcdef 09 240";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addFifoRead16_0()
{
  // we only care that we stick something on it
  m_pList->addFifoRead16(0xabcdef, 0x09, 240);

  vector<string> expected(1);
  expected[0] = "addFifoRead16 00abcdef 09 240";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addBlockWrite32_0()
{
  // we only care that we stick something on it
  uint32_t data[11] = {0, 10, 20, 30, 40, 50, 1, 2, 3, 4, 5};
  m_pList->addBlockWrite32(0xabcdef, 0x0b, data, 11);

  vector<string> expected(12);
  expected[0]  = "addBlockWrite32 00abcdef 0b 11";
  expected[1]  = "   0 : 00000000";
  expected[2]  = "   1 : 0000000a";
  expected[3]  = "   2 : 00000014";
  expected[4]  = "   3 : 0000001e";
  expected[5]  = "   4 : 00000028";
  expected[6]  = "   5 : 00000032";
  expected[7]  = "   6 : 00000001";
  expected[8]  = "   7 : 00000002";
  expected[9]  = "   8 : 00000003";
  expected[10] = "   9 : 00000004";
  expected[11] = "  10 : 00000005";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addBlockCountRead8_0()
{
  // we only care that we stick something on it
  m_pList->addBlockCountRead8(0xabcdef, 0x00abcd00, 0x09);

  vector<string> expected(1);
  expected[0]  = "addBlockCountRead8 00abcdef 00abcd00 09";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addBlockCountRead16_0()
{
  // we only care that we stick something on it
  m_pList->addBlockCountRead16(0xabcdef, 0x00abcd00, 0x09);

  vector<string> expected(1);
  expected[0]  = "addBlockCountRead16 00abcdef 00abcd00 09";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addBlockCountRead32_0()
{
  // we only care that we stick something on it
  m_pList->addBlockCountRead32(0xabcdef, 0x00abcd00, 0x09);

  vector<string> expected(1);
  expected[0]  = "addBlockCountRead32 00abcdef 00abcd00 09";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addMaskedCountBlockRead32_0()
{
  // we only care that we stick something on it
  m_pList->addMaskedCountBlockRead32(0xabcdef, 0x0b);

  vector<string> expected(1);
  expected[0]  = "addMaskedCountBlockRead32 00abcdef 0b";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addMaskedCountFifoRead32_0()
{
  // we only care that we stick something on it
  m_pList->addMaskedCountFifoRead32(0xabcdef, 0x0b);

  vector<string> expected(1);
  expected[0]  = "addMaskedCountFifoRead32 00abcdef 0b";

  CPPUNIT_ASSERT(expected == m_pList->getLog());
}

void CLoggingReadoutListTests::addDelay_0() 
{
  // we only care that we stick something on it
  m_pList->addDelay(23);

  vector<string> expected(1);
  expected[0]  = "addDelay 23";

  CPPUNIT_ASSERT(expected == m_pList->getLog());

}

void CLoggingReadoutListTests::addMarker_0() 
{
  // we only care that we stick something on it
  m_pList->addMarker(0xfedc);

  vector<string> expected(1);
  expected[0]  = "addMarker fedc";

  CPPUNIT_ASSERT(expected == m_pList->getLog());

}

#endif
