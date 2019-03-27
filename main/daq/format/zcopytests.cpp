// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CRingBuffer.h>
#include <CRingItem.h>


class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* m_pProducer;
  CRingBuffer* m_pConsumer;
public:
  void setUp() {
    try {
      CRingBuffer::remove("zcopytests");
    } catch(...) {}
    CRingBuffer::create("zcopytests");
    
    m_pProducer = new CRingBuffer("zcopytests", CRingBuffer::producer);
    m_pConsumer = new CRingBuffer("zcopytests", CRingBuffer::consumer);
  }
  void tearDown() {
    delete m_pProducer;
    delete m_pConsumer;
    CRingBuffer::remove("zcopytests");
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
