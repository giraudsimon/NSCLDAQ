// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CRingBuffer.h>

#include "testcommon.h"
#include "stdintoringUtils.h"

using namespace std;

class stdin2ringUtilsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(stdin2ringUtilsTest);
  CPPUNIT_TEST(integerize_1);
  CPPUNIT_TEST(integerize_2);
  CPPUNIT_TEST(integerize_3);
  CPPUNIT_TEST(integerize_4);
  
  CPPUNIT_TEST(size_1);
  CPPUNIT_TEST(size_2);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* m_pProducer;
  CRingBuffer* m_pConsumer;
public:
  void setUp() {
    m_pProducer = CRingBuffer::createAndProduce(uniqueRing("urllocal"));
    m_pConsumer = new CRingBuffer(uniqueRing("urllocal"));
  }
  void tearDown() {
    delete m_pProducer;
    delete m_pConsumer;
    CRingBuffer::remove(uniqueRing("urllocal"));  
  }
protected:
  void integerize_1();
  void integerize_2();
  void integerize_3();
  void integerize_4();
  
  void size_1();
  void size_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(stdin2ringUtilsTest);

// Set/get proxy ring size:

void stdin2ringUtilsTest::integerize_1()
{
  //  Straight number becomes a straight number:
  
  int result = integerize("1234");
  EQ(1234, result);
}

void stdin2ringUtilsTest::integerize_2()
{
  // k suffix
  
  int result = integerize("666k");
  EQ(666*1024, result);
}

void stdin2ringUtilsTest::integerize_3()
{
  // m suffix:
  
  int result = integerize("44m");
  EQ(44*1024*1024, result);
}
void stdin2ringUtilsTest::integerize_4()
{
  // g suffix.
  
  int result = integerize("1g");
  EQ(1024*1024*1024, result);
}

void stdin2ringUtilsTest::size_1()
{
  header h = {1234, 1};          // no swap required
  uint32_t result = computeSize(&h);
  EQ(uint32_t(1234), result);
}
void stdin2ringUtilsTest::size_2()
{
  // requires swap:
  
  header h = {0x1230000, 0x10000};
  uint32_t result = computeSize(&h);
  EQ(uint32_t(0x2301), result);
}