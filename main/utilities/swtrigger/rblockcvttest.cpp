// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "MessageTypes.h"
#include "RingItemBlockConverter.h"
#include <CRingItem.h>
#include <CPhysicsEventItem.h>
#include <DataFormat.h>

#include <CRingBlockReader.h>
#include <stdlib.h>
#include <string.h>





class rblockcvtTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rblockcvtTest);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(oneItem);
  CPPUNIT_TEST(severalItems);
  CPPUNIT_TEST_SUITE_END();


private:
  RingItemBlockConverter* m_pTestObj;
public:
  void setUp() {
    m_pTestObj = new RingItemBlockConverter;
  }
  void tearDown() {
    delete m_pTestObj;
    m_pTestObj = nullptr;
  }
protected:
  void empty();
  void oneItem();
  void severalItems();
private:
  CRingBlockReader::pDataDescriptor createDescriptor(
    std::list<CRingItem*>& items
  );
  void addRingItem(
    CRingBlockReader::DataDescriptor& d, CRingItem& r  
  );
  void freeDescriptor(CRingBlockReader::pDataDescriptor p);
};


// Utilities for building data descriptors.

// Create a data descriptor for a list of ring items.
// The ring items are copied to malloc'd storage and the
// descriptor itself is malloced too.

CRingBlockReader::pDataDescriptor
rblockcvtTest::createDescriptor(std::list<CRingItem*>& items)
{
  CRingBlockReader::pDataDescriptor result =
    static_cast<CRingBlockReader::pDataDescriptor>(malloc(
      sizeof(CRingBlockReader::DataDescriptor)  
    ));
    result->s_nItems = 0;
    result->s_nBytes = 0;
    result->s_pData  = nullptr;
    
    for (auto p : items) {
      addRingItem(*result, *p);  
    }
    return result;
}
// Add a ring item to a data block.

void
rblockcvtTest::addRingItem(
  CRingBlockReader::DataDescriptor& d, CRingItem& r
)
{
  // Increase the storage size:
  
  pRingItem pItem = r.getItemPointer();
  size_t totalSize = d.s_nBytes + pItem->s_header.s_size;
  d.s_pData = realloc(d.s_pData, totalSize);

  // Append the ring item:
  
  uint8_t* p = static_cast<uint8_t*>(d.s_pData);
  p += d.s_nBytes;
  memcpy(p, pItem, pItem->s_header.s_size);
  
  // Do the book keeping:
  
  d.s_nItems++;
  d.s_nBytes += pItem->s_header.s_size;
  
}
// Get rid of the data and the descriptor:

void
rblockcvtTest::freeDescriptor(CRingBlockReader::pDataDescriptor p)
{
  free(p->s_pData);
  free(p);
}

CPPUNIT_TEST_SUITE_REGISTRATION(rblockcvtTest);


// An empty descriptor results in an end message... no data parts.
void rblockcvtTest::empty() {
  std::pair<size_t, void*> nullItem = {0, nullptr}; 
  MessageType::Message m = (*m_pTestObj)(nullItem);
  
  EQ(MessageType::END_ITEM,  m.s_messageType);
  EQ(size_t(0), m.s_dataParts.size());
}
// A descriptor with one item we get PROCESS_ITEM a message part containing
// a pointer to uint32_t that's got 1. and a pointer to a data descriptor

void rblockcvtTest::oneItem()
{
  CPhysicsEventItem item;          // no body header.
  uint32_t* p = reinterpret_cast<uint32_t*>(item.getBodyCursor());
  *p++ = 0;
  item.setBodyCursor(p);
  item.updateSize();
  
  std::list<CRingItem*> items = {&item};
  
  CRingBlockReader::pDataDescriptor pD = createDescriptor(items);
  std::pair<size_t, void*> pDesc =
    {sizeof(CRingBlockReader::DataDescriptor), pD};
    
  MessageType::Message m = (*m_pTestObj)(pDesc);
  
  EQ(MessageType::PROCESS_ITEM, m.s_messageType);
  EQ(size_t(2), m.s_dataParts.size());
  
  // First one should be a pointer to a uint32_t containing the value 1.
  
  std::pair<size_t, void*> number = m.s_dataParts.front();
  std::pair<size_t, void*> array  = m.s_dataParts.back();
  
  uint32_t* pN = static_cast<uint32_t*>(number.second);
  EQ(sizeof(uint32_t), number.first);
  EQ(uint32_t(1), *pN);
  
  // The second one is a pointer to an array of 1 pointers to ring items.
  
  EQ(sizeof(void*), array.first);
  
  pRingItem *ppR = static_cast<pRingItem*>(array.second);
  pRingItem pR   = *ppR;
  
  EQ(PHYSICS_EVENT, pR->s_header.s_type);
  EQ(uint32_t(0), pR->s_body.u_noBodyHeader.s_mbz);
  uint32_t* body = reinterpret_cast<uint32_t*>(pR->s_body.u_noBodyHeader.s_body);
  EQ(uint32_t(0), *body);

  
  freeDescriptor(pD);
  
}
void rblockcvtTest::severalItems()
{
  // Make a list of ring items that have a uint32_t body that has as its' value
  // the ring item seq. number.
  
  std::list<CRingItem*> items;
  for (int i =0; i < 10; i++) {
    CRingItem* pItem = new CPhysicsEventItem;
    uint32_t* p = static_cast<uint32_t*>(pItem->getBodyCursor());
    *p++ = i;
    pItem->setBodyCursor(p);
    pItem->updateSize();
    
    items.push_back(pItem);
  }
  // Make the data descriptor and create the message:
  
  CRingBlockReader::pDataDescriptor pD = createDescriptor(items);
  std::pair<size_t, void*> pDesc =
    {sizeof(CRingBlockReader::DataDescriptor), pD};
  
  MessageType::Message m = (*m_pTestObj)(pDesc);
  
  // Should be a PROCESS_ITEM with two data parts.
  // The first one points to the integer 10, while the
  // second one has 10 pointers to the ring items we made.
  
  EQ(MessageType::PROCESS_ITEM, m.s_messageType);
  EQ(size_t(2), m.s_dataParts.size());
  
  std::pair<uint32_t, void*> count = m.s_dataParts.front();
  m.s_dataParts.pop_front();
  
  uint32_t* pCount  = static_cast<uint32_t*>(count.second);
  EQ(uint32_t(10), *pCount);
  
  std::pair<uint32_t, void*> pointers = m.s_dataParts.front();
  EQ(uint32_t(10*sizeof(void*)), pointers.first);
  
  pRingItem* ppItems = static_cast<pRingItem*>(pointers.second);
  for (int i = 0; i < 10; i++) {
    pRingItem pRItem = *ppItems++;
    uint32_t* pPayload = reinterpret_cast<uint32_t*>(
      pRItem->s_body.u_noBodyHeader.s_body
    );
    EQ(uint32_t(i), *pPayload);
  }
  
  freeDescriptor(pD);
  for (auto p : items) {
    delete p;
  }
}