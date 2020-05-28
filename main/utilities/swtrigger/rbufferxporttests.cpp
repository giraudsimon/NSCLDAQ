// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CRingBufferTransport.h"
#include <CRingBuffer.h>
#include <CRingBufferChunkAccess.h>
#include <DataFormat.h>

#include <sys/uio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

const std::string ringName("rbufferXportTest");

class rbufxportTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rbufxportTest);
  CPPUNIT_TEST(write_1);
  CPPUNIT_TEST(write_2);
  CPPUNIT_TEST(write_3);
  
  CPPUNIT_TEST(read_1);
  CPPUNIT_TEST(read_2);
  CPPUNIT_TEST(read_3);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer*    m_produceTo;
  CRingBuffer*    m_consumeFrom;
  CRingBufferChunkAccess*  m_reader;
  
  CRingBufferTransport*  m_producer;
  CRingBufferTransport*  m_consumer;
public:
  void setUp() {
    try {
      CRingBuffer::remove(ringName);   // Clean up the ring buffer itself.
    } catch (...) {}

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
    try {
      CRingBuffer::remove(ringName);   // Clean up the ring buffer itself.
    } catch (...) {}
  }
protected:
  void write_1();
  void write_2();
  void write_3();
  
  void read_1();
  void read_2();
  void read_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rbufxportTest);

void rbufxportTest::write_1() {   // Read throws logic_error.
  void* pData;
  size_t s;
  CPPUNIT_ASSERT_THROW(
    m_producer->recv(&pData, s),
    std::logic_error
  );
}
void rbufxportTest::write_2() {   // Write an item:
  RingItemHeader hdr;
  hdr.s_size = sizeof(hdr);
  hdr.s_type = PHYSICS_EVENT;
  
  iovec v;
  v.iov_base = &hdr;
  v.iov_len = hdr.s_size;
  m_producer->send(&v, 1);
  
  // Should be able to get this from the ringbuffer directly:
  
  RingItemHeader got;
  size_t nbytes = m_consumeFrom->get(&got, sizeof(got));
  EQ(nbytes, sizeof(got));
  EQ(uint32_t(sizeof(got)), got.s_size);
  EQ(PHYSICS_EVENT, got.s_type);
}

void rbufxportTest::write_3()          // Multipart write.
{
  // We'll do a gather store using these three bits and pieces.
  RingItemHeader hdr;                   
  uint32_t       mbz(0);
  uint32_t       data[32];
  for (int i = 0; i < sizeof(data)/sizeof(uint32_t); i++) {
    data[i] = i;
  }
  
  hdr.s_type = PHYSICS_EVENT;
  hdr.s_size = sizeof(hdr) + sizeof(uint32_t) + sizeof(data);
  
  iovec v[3];
  v[0].iov_base = &hdr;
  v[0].iov_len  = sizeof(hdr);
  
  v[1].iov_base = &mbz;
  v[1].iov_len  = sizeof(uint32_t);
  
  v[2].iov_base = data;
  v[2].iov_len  = sizeof(data);
  
  m_producer->send(v, 3);
  
  // Should be able to get a ring item
  
  uint8_t buffer[256];             // Should be more than enough but:
  ASSERT(hdr.s_size < sizeof(buffer));
  
  size_t nBytes = m_consumeFrom->get(buffer, sizeof(buffer), hdr.s_size, 0);
  EQ(size_t(hdr.s_size), nBytes);
  
  pRingItem pItem = reinterpret_cast<pRingItem>(buffer);
  EQ(uint16_t(PHYSICS_EVENT),itemType(pItem));
  EQ(hdr.s_size, itemSize(pItem));
  EQ(0, hasBodyHeader(pItem));
  uint32_t* pBody = reinterpret_cast<uint32_t*>(bodyPointer(pItem));
  
  for (int i = 0; i < sizeof(data)/sizeof(uint32_t); i++ ) {
    EQ(data[i], pBody[i]);
  }
  
}
void rbufxportTest::read_1()           // can't write the read xport
{
  iovec v;
  v.iov_base = nullptr;
  v.iov_len  = 0;
  CPPUNIT_ASSERT_THROW(
    m_consumer->send(&v, 1),
    std::logic_error
  );
}
void rbufxportTest::read_2()           // Ring buffer header.
{
  RingItemHeader hdr;
  hdr.s_size = sizeof(hdr);
  hdr.s_type = PHYSICS_EVENT;
  
  iovec v;
  v.iov_base = &hdr;
  v.iov_len = hdr.s_size;
  m_producer->send(&v, 1);

  void* pData;
  size_t nBytes;
  m_consumer->recv(&pData, nBytes);
  
  pRingItemHeader p = reinterpret_cast<pRingItemHeader>(pData);
  EQ(hdr.s_size, p->s_size);
  EQ(hdr.s_type, p->s_type);
  
  free(pData);
}
void rbufxportTest::read_3()        // Ring buffer with counting value.
{
  RingItemHeader hdr;
  hdr.s_type = PHYSICS_EVENT;
  hdr.s_size = sizeof(hdr) + 2 * sizeof(uint32_t);
  uint32_t       mbz(0);
  uint32_t       payload;
  
  iovec v[3];
  v[0].iov_base = &hdr;
  v[0].iov_len = sizeof(hdr);
  v[1].iov_base = &mbz;
  v[1].iov_len = sizeof(uint32_t);
  v[2].iov_base = &payload;
  v[2].iov_len  = sizeof(payload);
  
  for (int i =0; i < 10; i++) {
    payload = i;
    m_producer->send(v, 3);
  }
  
  for (int i =0; i < 10; i++) {
    size_t nBytes;
    void*  pData;
    m_consumer->recv(&pData, nBytes);
    
    pRingItem p = reinterpret_cast<pRingItem>(pData);
    EQ(hdr.s_size, itemSize(p));
    EQ(uint16_t(hdr.s_type), itemType(p));
    uint32_t* pD = reinterpret_cast<uint32_t*>(bodyPointer(p));
    EQ(uint32_t(0), *pD);   pD++;
    EQ(uint32_t(i), *pD);
    
    free(pData);
  }
}
