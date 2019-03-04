// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CRingBuffer.h>
#include <ringbufint.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "testcommon.h"

using namespace std;

// Default name of shared memory special file:




class DiffTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DiffTests);
  CPPUNIT_TEST(simple);
  CPPUNIT_TEST(justwrapped);
  CPPUNIT_TEST(full);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* m_pProducer;
  CRingBuffer* m_pConsumer;
 
  pRingBuffer        m_pRing;
  pRingHeader        m_pHeader;
  pClientInformation m_pPut;
  pClientInformation m_pGet;
  
  std::string SHM_TESTFILE;
  
public:

  void setUp() {
    SHM_TESTFILE= uniqueRing("difftest");
    m_pProducer = 0;
    m_pConsumer = 0;
    CRingBuffer::create(string(SHM_TESTFILE));
    m_pProducer = new CRingBuffer(string(SHM_TESTFILE), CRingBuffer::producer);
    m_pConsumer = new CRingBuffer(string(SHM_TESTFILE), CRingBuffer::consumer);

    m_pRing   = (pRingBuffer)mapRingBuffer(SHM_TESTFILE.c_str());
    m_pHeader = reinterpret_cast<pRingHeader>(m_pRing);
    m_pPut    = &(m_pRing->s_producer);
    m_pGet    = m_pRing->s_consumers;

  }
  void tearDown() {
    if (m_pProducer) {
      munmap(m_pRing, m_pRing->s_header.s_topOffset+1);
      delete m_pProducer;
      delete m_pConsumer;
    }
    try {
      CRingBuffer::remove(SHM_TESTFILE);
    }
    catch (...) {}
  }
protected:
  void simple();
  void justwrapped();
  void full();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiffTests);

// Simple differences.
//   if pointers are the same, the buffer is empty, all but one avail.
//   If put pointer is ahead of get pointer by 1, there's  one byte in the buffer.
//   all but 2 avail.
void DiffTests::simple() {
  EQ((size_t)0, m_pConsumer->availableData());
  EQ(m_pHeader->s_dataBytes-1, m_pProducer->availablePutSpace());

  m_pPut->s_offset++;

  EQ((size_t)1, m_pConsumer->availableData());
  EQ(m_pHeader->s_dataBytes-2, m_pProducer->availablePutSpace());
  
}

//
// Test the case where we've just wrapped
//
void DiffTests::justwrapped()
{
  m_pGet->s_offset = m_pHeader->s_topOffset;
  EQ((size_t)1, m_pConsumer->availableData());
  EQ(m_pHeader->s_dataBytes-2, m_pProducer->availablePutSpace());


}

// If the producer is just behind the consumer we're full.
// there are two cases.
// in the middle of the buffer, and
// across the warp.
//

void DiffTests::full()
{

  // just behind with no wrap


  m_pGet->s_offset++;		// Should now be full.
  EQ(m_pHeader->s_dataBytes-1, m_pConsumer->availableData());
  EQ((size_t)0, m_pProducer->availablePutSpace());


  // just behind with a wrap:

  m_pGet->s_offset--;		// Back to the beginning.
  m_pPut->s_offset = m_pHeader->s_topOffset; // just behind at the end..
  EQ(m_pHeader->s_dataBytes-1, m_pConsumer->availableData());
  EQ((size_t)0, m_pProducer->availablePutSpace());

}
