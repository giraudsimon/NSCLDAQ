// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include "CSender.h"
#include "CReceiver.h"
#include "CZMQServerTransport.h"
#include "CZMQClientTransport.h"

#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#inclue <stdint.h>
#include <zmq.hpp>

static const std::string service("inproc://receivertest");



class receiverTest : public CppUnit::TestFixture {  // ZMQ Push/Pull.
  CPPUNIT_TEST_SUITE(receiverTest);
  CPPUNIT_TEST(recv_1);
  CPPUNIT_TEST_SUITE_END();


private:
  CSender*       m_pSender;
  CReceiver*     m_pTestObj;
  
  CZMQServerTransport* m_pSenderTransport;
  CZMQClientTransport* m_pReceiverTransport;
public:
  void setUp() {
    m_pSenderTransport = new CZMQServerTransport(service.c_str(), ZMQ_PUSH);
    m_pReceiverTransport= new CZMQClientTransport(service.c_str(), ZMQ_PULL);
    
    m_pSender = new CSender(*m_pSenderTransport);
    m_pTestObj = new CReceiver(*m_pReceiverTransport);
  }
  void tearDown() {
    delete m_pSender;
    delete m_pTestObj;
    delete m_pSenderTransport;
    delete m_pReceiverTransport;
  }
protected:
  void recv_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(receiverTest);

void receiverTest::recv_1() {              // Single part message:
  uint8_t  msg[100];
  for (int i =0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  
  m_pSender->sendMessage(msg, sizeof(msg));
  
  void* pData
  size_t nBytes;
  m_pTestObj->getMessage(&pData, nBytes);
  EQ(sizeof(msg), nBytes);
  EQ(0, memcmp(msg, pData, sizeof(msg)));
  
  free(pData);
}
