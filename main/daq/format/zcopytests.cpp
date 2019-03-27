// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <CRingBuffer.h>
#define private public
#include "CRingItem.h"
#undef private
#include "DataFormat.h"
#include "CAllButPredicate.h"


#include <string.h>
#include <stdexcept>

class zcopytest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zcopytest);
  CPPUNIT_TEST(construct_1);
  CPPUNIT_TEST(commit_1);
  CPPUNIT_TEST(commit_2);
  
  CPPUNIT_TEST(construct_2);
  
  CPPUNIT_TEST(commit_3);
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
  void construct_1();
  
  void commit_1();
  void commit_2();
  
  void construct_2();
  
  void commit_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zcopytest);

void zcopytest::construct_1() {  // construction doesn't commit anything.
  CRingItem zcopy(PHYSICS_EVENT, 0x1234, 1, 0, 128, m_pProducer);
  EQ(size_t(0), m_pConsumer->availableData());
}
void zcopytest::commit_1()      // Comitting puts data into a ring buffer.
{
  CAllButPredicate all;
  
  CRingItem zcopy(PHYSICS_EVENT, 0x1234, 1, 0, 128, m_pProducer);
  zcopy.commitToRing(*m_pProducer);
  EQ(size_t(zcopy.getItemPointer()->s_header.s_size), m_pConsumer->availableData());
  
  CRingItem* pGotten = CRingItem::getFromRing(*m_pConsumer, all);

  EQ(
    0,
    memcmp(
      zcopy.getItemPointer(), pGotten->getItemPointer(),
      zcopy.getItemPointer()->s_header.s_size)
  );
  
  delete pGotten;
}
void zcopytest::commit_2()          // Put something in the ring item:
{
  CAllButPredicate all;
  

  CRingItem zcopy(PHYSICS_EVENT, 0x1234, 1, 0, 128, m_pProducer);
  uint32_t* p = static_cast<uint32_t*>(zcopy.getBodyCursor());
  for (int i  = 0 ; i < 100; i++) {
    *p++ = i;
  }
  zcopy.setBodyCursor(p);
  zcopy.updateSize();
  
  zcopy.commitToRing(*m_pProducer);
  EQ(size_t(zcopy.getItemPointer()->s_header.s_size), m_pConsumer->availableData());
  
  CRingItem* pGotten = CRingItem::getFromRing(*m_pConsumer, all);

  EQ(
    0,
    memcmp(
      zcopy.getItemPointer(), pGotten->getItemPointer(),
      zcopy.getItemPointer()->s_header.s_size)
  );
  
  delete pGotten;
  
}

void zcopytest::construct_2()
{
  CAllButPredicate all;
  
  // Push the pointers so that construction must wrap
  // this should result in a ring item that's not zero copy:
  
  size_t nBytes = m_pProducer->availablePutSpace();
  size_t skip   = nBytes - sizeof(uint16_t);
  m_pProducer->skip(skip);
  m_pConsumer->skip(skip);
  
  CRingItem nozcopy(PHYSICS_EVENT, 0x1234, 1, 0, 128, m_pProducer);
  ASSERT(!nozcopy.m_fZeroCopy);
  nozcopy.commitToRing(*m_pProducer);
  
  CRingItem* pGotten = CRingItem::getFromRing(*m_pConsumer, all);
  EQ(
    0,
    memcmp(
      nozcopy.getItemPointer(), pGotten->getItemPointer(),
      nozcopy.getItemPointer()->s_header.s_size)
  );
  delete pGotten;
}
void zcopytest::commit_3()
{
  // committing on a different ring is bad:
  
  CRingItem zcopy(PHYSICS_EVENT, 0x1234, 1, 0, 128, m_pProducer);
  
  /// The exception for comitting to a consumer is different so this is
  // a valid test:
  
  CPPUNIT_ASSERT_THROW(
    zcopy.commitToRing(*m_pConsumer),
    std::logic_error
  );
}