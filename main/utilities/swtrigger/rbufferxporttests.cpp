// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CRingBufferTransport.h"
#include <CRingBuffer.h>
#include <CRingBufferChunkAccess.h>

#include <sys/uio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

const std::string ringName("rbufferXportTest");

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer*    m_produceTo;
  CRingBuffer*    m_consumeFrom;
  CRingBufferChunkAccess*  m_reader;
  
  CRingBufferTransport*  m_producer;
  CRingBufferTransport*  m_consumer;
public:
  void setUp() {
    m_produceTo   = CRingBuffer::createAndProduce(ringName);
    m_consumeFrom = new CRingBuffer(ringName);    // Default is consumer.
    m_reader      = new CRingBufferChunkAccess(m_consumeFrom);
    
    m_producer = new CRingBufferTransport(*m_produceTo);
    m_consumer = new CRingBufferTransport(*m_reader);
    
  }
  void tearDown() {
    delete m_producer;               // Deletes the production ring buffer obj.
    delete m_consumer;               // deletes the chunk accessor but we need
    delete m_consumeFrom;            // to delete the CRingBuffer.
    
    CRingBuffer::remove(ringName);   // Clean up the ring buffer itself.
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
