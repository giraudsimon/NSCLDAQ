// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "SingleRingItemConverter.h"
#include "DataFormat.h"
#include <CPhysicsEventItem.h>
#include "MessageTypes.h"
#include <CRingItemFactory.h>


class singleritemconvTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(singleritemconvTest);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(physics);
  CPPUNIT_TEST_SUITE_END();


private:
  SingleRingItemConverter* m_pTestObj;
public:
  void setUp() {
    m_pTestObj = new SingleRingItemConverter;
  }
  void tearDown() {
    delete m_pTestObj;
    m_pTestObj = nullptr;
  }
protected:
  void empty();
  void physics();
};

CPPUNIT_TEST_SUITE_REGISTRATION(singleritemconvTest);


// Empty pairs result in a end item.

void singleritemconvTest::empty() {
  std::pair<size_t, void*> item = {size_t(0), (void*)(nullptr)};
  MessageType::Message msg = (*m_pTestObj)(item);
  
  EQ(MessageType::END_ITEM, msg.s_messageType);
  ASSERT(msg.s_dataParts.empty());
}
// Non empty pairs result in a PROCESS_ITEM with the
// ring item as the payload.

void singleritemconvTest::physics()
{
  CPhysicsEventItem i(0x1234, 1, 0);     // With body header too
  // Put a counting pattern in:
  
  uint16_t* p = static_cast<uint16_t*>(i.getBodyCursor());
  for (int i =0; i < 16; i++) {
    *p++ = i;
  }
  i.setBodyCursor(p);
  i.updateSize();
  
  pRingItem pRawItem = i.getItemPointer();
  std::pair<size_t, void*> item = {size_t(pRawItem->s_header.s_size), pRawItem};
  
  MessageType::Message msg = (*m_pTestObj)(item);
  
  EQ(MessageType::PROCESS_ITEM, msg.s_messageType);
  EQ(size_t(1), msg.s_dataParts.size());
  std::pair<uint32_t, void*> seg1 = msg.s_dataParts.front();
  
  EQ(item.first, size_t(seg1.first));
  EQ(item.second, seg1.second);
  
}
