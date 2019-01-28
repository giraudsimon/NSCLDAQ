// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "fakedatasource.h"
#include "ZMQPushDataSource.h"
#include "swFilterRingBlockDataSource.h"
#include "RingItemBlockConverter.h"

#include <DataFormat.h>
#include <CPhysicsEventItem.h>


#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <zmq.hpp>


static const std::string uri("inproc://test");


class zmqpushblocktest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqpushblocktest);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(oneItem);
  CPPUNIT_TEST(afew);
  CPPUNIT_TEST_SUITE_END();


private:CFakeRingBlockReader*   m_pDataSource;
  RingItemBlockConverter* m_pConverter;
  swFilterRingBlockDataSource* m_pSrc;
  ZMQPushDataSource*      m_pTestObj;
  
  zmq::socket_t*          m_pSocket;
public:
  void setUp() {
    m_pDataSource = new CFakeRingBlockReader;
    m_pConverter  = new RingItemBlockConverter;
    m_pSrc        = new swFilterRingBlockDataSource(*m_pDataSource, 8192);
    m_pTestObj    = new ZMQPushDataSource("test", uri, m_pSrc, m_pConverter);
    
    m_pSocket = static_cast<zmq::socket_t*>(m_pTestObj->connectAsSink());
    usleep(1000*100);            // Let the connection work.
  }
  void tearDown() {
    m_pTestObj->closeSink(m_pSocket);
    
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
  void oneItem();
  void afew();
private:
  void putItems(size_t n);    // Put n physics items in m_pDataSource with seq.
};

CPPUNIT_TEST_SUITE_REGISTRATION(zmqpushblocktest);


///////////////////////////////////////////////////////////////////////////
// utilities:

void
zmqpushblocktest::putItems(size_t n)
{
  for (int i = 0; i < n; i ++) {
    CPhysicsEventItem item(i, 123, 0);
    uint32_t* p = static_cast<uint32_t*>(item.getBodyCursor());
    *p++ = i;
    item.setBodyCursor(p);
    item.updateSize();
    
    pRingItem pItem = item.getItemPointer();
    m_pDataSource->addData(pItem, pItem->s_header.s_size);
  }
}

// Running with no data will immediately send us an END_ITEM message.
// 
void zmqpushblocktest::empty() {
  m_pTestObj->start();           // Should send an END_ITEM:
  
  std::list<zmq::message_t*> msg = getMessage(m_pSocket);
  
  EQ(size_t(1), msg.size());
  zmq::message_t* e = msg.front();
  EQ(sizeof(uint32_t), e->size());
  
  uint32_t code;
  memcpy(&code, e->data(), sizeof(uint32_t));
  EQ(MessageType::END_ITEM, code);
  
  deleteMessage(msg);
}
// Single item - data item with the item then end item.

void zmqpushblocktest::oneItem()
{
  putItems(1);
  m_pTestObj->start();
  
  std::list<zmq::message_t*> data = getMessage(m_pSocket);
  std::list<zmq::message_t*> msg  = getMessage(m_pSocket);
  
  EQ(size_t(3), data.size());    // code count and descriptor.
  std::list<zmq::message_t*>::iterator p = data.begin();
  zmq::message_t* type = *p++;
  EQ(sizeof(uint32_t), type->size());
  uint32_t* pCode = static_cast<uint32_t*>(type->data());
  EQ(MessageType::PROCESS_ITEM, *pCode);
  
  zmq::message_t* count = *p++;
  EQ(sizeof(uint32_t), count->size());
  pCode = static_cast<uint32_t*>(type->data());
  EQ(uint32_t(1), *pCode);
  
  zmq::message_t* payload = *p++;
  EQ(sizeof(pRingItem*), payload->size());
  pRingItem* pData = static_cast<pRingItem*>(payload->data());
  EQ(PHYSICS_EVENT, pData[0]->s_header.s_type);
  EQ(uint64_t(0), pData[0]->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
  EQ(uint32_t(123), pData[0]->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
  EQ(uint32_t(0), pData[0]->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
  uint32_t* pBody =
    reinterpret_cast<uint32_t*>(pData[0]->s_body.u_hasBodyHeader.s_body);
  EQ(uint32_t(0), *pBody);
  
  EQ(size_t(1), msg.size());
  zmq::message_t* e = msg.front();
  EQ(sizeof(uint32_t), e->size());
  
  uint32_t code;
  memcpy(&code, e->data(), sizeof(uint32_t));
  EQ(MessageType::END_ITEM, code);
  
  
  deleteMessage(data);
  deleteMessage(msg);
}
void zmqpushblocktest::afew()
{
  // We're going to assume that we fit all the items in the readsize for now.
  // elsewhere we've tested the blockspanning works ok.
  
  putItems(10);
  m_pTestObj->start();
    
  std::list<zmq::message_t*> data = getMessage(m_pSocket);
  std::list<zmq::message_t*> msg  = getMessage(m_pSocket);
  
  EQ(size_t(3), data.size());    // code count and pointer array.
  std::list<zmq::message_t*>::iterator p = data.begin();
  zmq::message_t* type = *p++;
  EQ(sizeof(uint32_t), type->size());
  uint32_t* pCode = static_cast<uint32_t*>(type->data());
  EQ(MessageType::PROCESS_ITEM, *pCode);
  
  zmq::message_t* count = *p++;
  EQ(sizeof(uint32_t), count->size());
  pCode = static_cast<uint32_t*>(count->data());
  EQ(uint32_t(10), *pCode);             // 10 events.
  
  zmq::message_t* payload = *p++;
  EQ(sizeof(pRingItem*) *10, payload->size());
  pRingItem* pData = static_cast<pRingItem*>(payload->data());
  
  for (int i = 0; i < 10; i++) {
    
    EQ(PHYSICS_EVENT, pData[i]->s_header.s_type);
    EQ(uint64_t(i), pData[i]->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(123), pData[i]->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), pData[i]->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    uint32_t* pBody =
      reinterpret_cast<uint32_t*>(pData[i]->s_body.u_hasBodyHeader.s_body);
    EQ(uint32_t(i), *pBody);
  }
  
  EQ(size_t(1), msg.size());
  zmq::message_t* e = msg.front();
  EQ(sizeof(uint32_t), e->size());
  
  uint32_t code;
  memcpy(&code, e->data(), sizeof(uint32_t));
  EQ(MessageType::END_ITEM, code);
  
  
  deleteMessage(data);
  deleteMessage(msg);
}