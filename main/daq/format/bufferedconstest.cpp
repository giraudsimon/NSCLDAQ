// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <DataFormat.h>
#include <CRingBuffer.h>
#include "CBufferedRingItemConsumer.h"

static std::string ringname("bconstest");



class bconstest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(bconstest);
  CPPUNIT_TEST(singleItem);
  CPPUNIT_TEST(twoItem);
  CPPUNIT_TEST(split2_1);
  CPPUNIT_TEST(split2_2);
  CPPUNIT_TEST(split2_3);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* m_pProducer;
  CRingBuffer* m_pConsumer;
  CBufferedRingItemConsumer* m_pTestObj;
public:
  void setUp() {
    try {
      CRingBuffer::remove(ringname);
    } catch(...) {}
    CRingBuffer::create(ringname);
    
    m_pProducer = CRingBuffer::createAndProduce(ringname);
    m_pConsumer = new CRingBuffer(ringname);
    
    m_pTestObj = new CBufferedRingItemConsumer(*m_pConsumer);
    
  }
  void tearDown() {
    delete m_pTestObj;
    delete m_pConsumer;
    delete m_pProducer;
    CRingBuffer::remove(ringname);
  }
protected:
  void singleItem();
  void twoItem();
  void split2_1();
  void split2_2();
  void split2_3();
private:
  void putItem(size_t bodySize);
  void putPartial(size_t size, size_t bytesToPut);
  void putRemainder(size_t size, size_t bytesAlreadyPut);
};

// Utility to put a ring item in the ringbuffer with a specific bodysize.
// for this test all items are physics_event.
void bconstest::putItem(size_t bodySize)
{
  size_t is = bodySize + sizeof(RingItemHeader);
  uint8_t item[is];
  pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(item);
  pH->s_size = is;
  pH->s_type = PHYSICS_EVENT;
  m_pProducer->put(item, is);
}
// Put part of a ring item. size - size of ring item bytesToPut -how much to put.

void bconstest::putPartial(size_t size, size_t bytesToPut)
{
  // We're going to make the full ring item and then just put what we're asked to.
  // we're trusting the caller to at least give us a header to work with
  
  uint8_t item[size];
  pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(item);
  
  pH->s_size = size;
  pH->s_type = PHYSICS_EVENT;
  
  m_pProducer->put(item, bytesToPut);
  
}
// Put the rest of a ring item

void bconstest::putRemainder(size_t size, size_t bytesAlreadyPut)
{
  uint8_t item[size];
  pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(item);
  
  pH->s_size = size;
  pH->s_type = PHYSICS_EVENT;
  
  size_t remainder = size - bytesAlreadyPut;
  uint8_t* p = item + bytesAlreadyPut;
  m_pProducer->put(p, remainder);
}

CPPUNIT_TEST_SUITE_REGISTRATION(bconstest);

void bconstest::singleItem() {
  putItem(100);
  
  void* pItem = m_pTestObj->get();
  pRingItemHeader p = static_cast<pRingItemHeader>(pItem);
  EQ(uint32_t(100 + sizeof(RingItemHeader)), p->s_size);
  EQ(PHYSICS_EVENT, p->s_type);
}
//Put two items
// Can get two out but only one fill.
void bconstest::twoItem() {
  putItem(100);
  putItem(200);
  pRingItemHeader p1 = static_cast<pRingItemHeader>(m_pTestObj->get());
  pRingItemHeader p2 = static_cast<pRingItemHeader>(m_pTestObj->get());
  
  EQ(size_t(1), m_pTestObj->getFills());
  
  EQ(uint32_t(100 + sizeof(RingItemHeader)), p1->s_size);
  EQ(uint32_t(200 + sizeof(RingItemHeader)), p2->s_size);
  
}
// two items split so only the size gets put in for the second.

void bconstest::split2_1()
{
  putItem(100);
  putPartial(200, sizeof(uint32_t));
  
  pRingItemHeader p1 = static_cast<pRingItemHeader>(m_pTestObj->get());
  EQ(uint32_t(100 + sizeof(RingItemHeader)), p1->s_size);

  putRemainder(200, sizeof(uint32_t));
  
  pRingItemHeader p2 = static_cast<pRingItemHeader>(m_pTestObj->get());
  
  EQ(size_t(2), m_pTestObj->getFills());   // needd to fill twice.
  
  EQ(uint32_t(200), p2->s_size);
  
}
// Two items split so that only part of the size of the second one is put.

void bconstest::split2_2()
{
  putItem(100);
  putPartial(200, 1);
  pRingItemHeader p1 = static_cast<pRingItemHeader>(m_pTestObj->get());
  EQ(uint32_t(100 + sizeof(RingItemHeader)), p1->s_size);

  putRemainder(200, 1);
  
  pRingItemHeader p2 = static_cast<pRingItemHeader>(m_pTestObj->get());
  
  EQ(size_t(2), m_pTestObj->getFills());   // needd to fill twice.
  
  EQ(uint32_t(200), p2->s_size); 
}

// Two items and part of the second beyond the size is put.

void bconstest::split2_3()
{
  putItem(100);
  putPartial(200, 100);
  pRingItemHeader p1 = static_cast<pRingItemHeader>(m_pTestObj->get());
  EQ(uint32_t(100 + sizeof(RingItemHeader)), p1->s_size);

  putRemainder(200, 100);
  
  pRingItemHeader p2 = static_cast<pRingItemHeader>(m_pTestObj->get());
  
  EQ(size_t(2), m_pTestObj->getFills());   // needd to fill twice.
  
  EQ(uint32_t(200), p2->s_size); 
}