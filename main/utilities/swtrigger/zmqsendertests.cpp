// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include "CSender.h"
#include "CZMQServerTransport.h"
#include "CZMQClientTransport.h"

#include <string>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.hpp>
#include <stdint.h>

std::string service("inproc://sendertests");


class sendertest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sendertest);
  CPPUNIT_TEST(sndmsg_1);           // Multipart with iovec.
  CPPUNIT_TEST(sndmsg_2);           // single part with data/size.
  CPPUNIT_TEST_SUITE_END();


private:
  CSender*   m_pTestObj;
  CZMQServerTransport* m_pSenderTransport;
  CZMQClientTransport* m_pReceiver;
  
public:
  void setUp() {              // USe push/pull.
    m_pSenderTransport = new CZMQServerTransport(service.c_str(), ZMQ_PUSH);
    m_pTestObj         = new CSender(*m_pSenderTransport);
    m_pReceiver        = new CZMQClientTransport(service.c_str(), ZMQ_PULL);
  }
  void tearDown() {
    delete m_pTestObj;
    delete m_pSenderTransport;
    delete m_pReceiver;
    
  }
protected:
  void sndmsg_1();
  void sndmsg_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sendertest);

void sendertest::sndmsg_1() {
  uint8_t part1[100];
  uint8_t part2[200];
  
  for (int i =0; i < sizeof(part1); i++) {
    part1[i];
  }
  for (int i =0; i < sizeof(part2); i++) {
    part2[i] = sizeof(part2) - i;
  }
  
  iovec v[2];
  v[0].iov_base = part1;
  v[0].iov_len  = sizeof(part1);
  v[1].iov_base = part2;
  v[1].iov_len  = sizeof(part2);
  
  m_pTestObj->sendMessage(v, 2);
  
  void* pData(0);
  size_t n;
  m_pReceiver->recv(&pData, n);
  
  EQ(sizeof(part1) + sizeof(part2), n);
  EQ(0, memcmp(part1, pData, sizeof(part1)));
  uint8_t* p2 = static_cast<uint8_t*>(pData);
  p2 += sizeof(part1);
  EQ(0, memcmp(part2, p2, sizeof(part2)));
  
  free(pData);
}

void sendertest::sndmsg_2()
{
  uint8_t part1[100];
  
  for (int i =0; i < sizeof(part1); i++) {
    part1[i];
  }
  
  m_pTestObj->sendMessage(part1, sizeof(part1));
  
  void* pData(0);
  size_t n;
  m_pReceiver->recv(&pData, n);
  
  EQ(sizeof(part1), n);
  EQ(0, memcmp(part1, pData, sizeof(part1)));  
}