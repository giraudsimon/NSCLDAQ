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
#include <stdint.h>
#include <sys/uio.h>
#include <zmq.hpp>

static const std::string service("inproc://receivertest");



class receiverTest : public CppUnit::TestFixture {  // ZMQ Push/Pull.
  CPPUNIT_TEST_SUITE(receiverTest);
  CPPUNIT_TEST(recv_1);
  CPPUNIT_TEST(recv_2);
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
  void recv_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(receiverTest);

void receiverTest::recv_1() {              // Single part message:
  uint8_t  msg[100];
  for (int i =0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  
  m_pSender->sendMessage(msg, sizeof(msg));
  
  void* pData;
  size_t nBytes;
  m_pTestObj->getMessage(&pData, nBytes);
  EQ(sizeof(msg), nBytes);
  EQ(0, memcmp(msg, pData, sizeof(msg)));
  
  free(pData);
}
void receiverTest::recv_2()
{
  uint8_t part1[100];
  uint8_t part2[200];
  for (int i = 0; i < sizeof(part1); i++) {
    part1[i] = i;
  }
  for (int i =0; i < sizeof(part2); i++) {
    part2[i] = sizeof(part2) - i;
  }
  iovec v[2];                    // parts descriptors.
  v[0].iov_base = part1;
  v[0].iov_len  = sizeof(part1);
  v[1].iov_base = part2;
  v[1].iov_len  = sizeof(part2);
  
  m_pSender->sendMessage(v, 2);
  
  void* pData;
  size_t nBytes;
  m_pTestObj->getMessage(&pData, nBytes);
  
  EQ(sizeof(part1) + sizeof(part2), nBytes);
  EQ(0, memcmp(part1, pData,sizeof(part1)));
  uint8_t* p2 = static_cast<uint8_t*>(pData);
  p2 += sizeof(part1);
  EQ(0, memcmp(part2, p2, sizeof(part2)));
  
  free(pData);
  
}