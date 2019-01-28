// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>


#include "fakedatasource.h"
#include "SingleRingItemConverter.h"
#include "swFilterRingDataSource.h"
#include "ZMQPushDataSource.h"

#include <CRingItem.h>
#include <DataFormat.h>

#include <zmq.hpp>
#include <list>
#include <string.h>
#include <stdlib.h>
#include <string>

static const std::string uri="inproc://test";

class zmqpushsrcTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqpushsrcTest);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(oneitem);
  CPPUNIT_TEST(manyitems);
  CPPUNIT_TEST_SUITE_END();


private:
  CFakeDataSource*         m_pDataSource;
  SingleRingItemConverter* m_pConverter;
  swFilterRingDataSource*  m_pSrc;
  ZMQPushDataSource*       m_pTestObj;
  
  zmq::socket_t*          m_pSocket;
public:
  void setUp() {
    m_pDataSource = new CFakeDataSource;
    m_pConverter  = new SingleRingItemConverter;
    m_pSrc        = new swFilterRingDataSource(*m_pDataSource);
    m_pTestObj    = new ZMQPushDataSource("source", uri, m_pSrc, m_pConverter);
    m_pSocket     = static_cast<zmq::socket_t*>(m_pTestObj->connectAsSink());
    usleep(1000*100);                // 100ms wait.
  }
  void tearDown() {
    
    m_pTestObj->closeSink(m_pSocket);
    
    // We assume the message get fully drained.
    
    delete m_pTestObj;
    delete m_pConverter;
    delete m_pSrc;
    delete m_pDataSource;
    
    m_pTestObj = nullptr;
    m_pConverter = nullptr;
    m_pSrc   = nullptr;
    m_pDataSource = nullptr;
  }
protected:
  void empty();
  void oneitem();
  void manyitems();
private:
  std::list<zmq::message_t*> getMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zmqpushsrcTest);

// Utilities:

std::list<zmq::message_t*> 
zmqpushsrcTest::getMessage()
{
  return ::getMessage(m_pSocket);
}


// If there's
void zmqpushsrcTest::empty() {
  // Get message parts from the message
  
  m_pTestObj->start();           // Start the thread.
  
  
  std::list<zmq::message_t*> msg = getMessage();
  
  // There should be one message part, it contains
  // a uint32_t with the value MessageType::END_ITEM
  
  EQ(size_t(1), msg.size());
  uint32_t code;
  EQ(sizeof(uint32_t), msg.front()->size());
  memcpy(&code, msg.front()->data(), sizeof(uint32_t));
  EQ(MessageType::END_ITEM, code);
  
  deleteMessage(msg);
  
}
// I can get a correct message with a single ring item.

void zmqpushsrcTest::oneitem()
{
  CRingItem* p =   new CRingItem(PHYSICS_EVENT);
  m_pDataSource->addItem(p);
  
  m_pTestObj->start();
  
  // should get two messages, one PROCESS_ITEM with the ring item
  // and one END_ITEM with no payload.
  
  std::list<zmq::message_t*> item = getMessage();
  std::list<zmq::message_t*> end  = getMessage();
  
  EQ(size_t(2), item.size());
  uint32_t code;
  EQ(sizeof(uint32_t), item.front()->size());
  memcpy(&code, item.front()->data(), sizeof(uint32_t));
  EQ(MessageType::PROCESS_ITEM, code);
  
  pRingItem pItem = static_cast<pRingItem>(item.back()->data());;

  EQ(PHYSICS_EVENT, pItem->s_header.s_type);
  EQ(sizeof(pRingItemHeader) + sizeof(uint32_t), size_t(pItem->s_header.s_size));
  EQ(uint32_t(0), pItem->s_body.u_noBodyHeader.s_mbz);
  
  EQ(sizeof(uint32_t), end.front()->size());
  memcpy(&code, end.front()->data(), sizeof(uint32_t));
  EQ(MessageType::END_ITEM, code);  
  
  deleteMessage(item);
  deleteMessage(end);
}

// Should be able to put many items.  In this case each physic item
// will have a serial number.

void zmqpushsrcTest::manyitems()
{
  for (int i =0; i < 10; i++) {
    CRingItem* p = new CRingItem(PHYSICS_EVENT);
    uint32_t* c = static_cast<uint32_t*>(p->getBodyCursor());
    *c++ = i;
    p->setBodyCursor(c);
    p->updateSize();
    m_pDataSource->addItem(p);
  }
  m_pTestObj->start();
  
  for (int i = 0; i < 10; i++) {
    std::list<zmq::message_t*> item = getMessage();
    
    EQ(size_t(2), item.size());
    uint32_t code;
    EQ(sizeof(uint32_t), item.front()->size());
    memcpy(&code, item.front()->data(), sizeof(uint32_t));
    EQ(MessageType::PROCESS_ITEM, code);
    
    pRingItem pItem = static_cast<pRingItem>(item.back()->data());;
  
    EQ(PHYSICS_EVENT, pItem->s_header.s_type);
    EQ(sizeof(pRingItemHeader) + 2*sizeof(uint32_t), size_t(pItem->s_header.s_size));
    EQ(uint32_t(0), pItem->s_body.u_noBodyHeader.s_mbz);
    EQ(uint32_t(i), *(uint32_t*)(pItem->s_body.u_noBodyHeader.s_body));
    
    
    deleteMessage(item);
  }
  std::list<zmq::message_t*> end  = getMessage();
  uint32_t code;
  EQ(sizeof(uint32_t), end.front()->size());
  memcpy(&code, end.front()->data(), sizeof(uint32_t));
  EQ(MessageType::END_ITEM, code);  
  
  deleteMessage(end);
}