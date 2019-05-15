// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CZMQClientTransport.h"
#include "CZMQServerTransport.h"

#include <zmq.hpp>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

static const std::string service="inproc://zmqclienttests";



class zmqcliTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqcliTest);
  CPPUNIT_TEST(sendrcv_1);
  CPPUNIT_TEST_SUITE_END();


private:
  CZMQClientTransport*  m_pTestObj;
  CZMQServerTransport*  m_pSender;             // Testsed in zmqsvrxporttests.
public:
  void setUp() {
    
    m_pSender  = new CZMQServerTransport(service.c_str(), ZMQ_PUSH);
    usleep(100);
    m_pTestObj = new CZMQClientTransport(service.c_str(), ZMQ_PULL);
  
  }
  void tearDown() {
    delete m_pTestObj;
    delete m_pSender;
    usleep(100);                     // Let the transport run down.
  }
protected:
  void sendrcv_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zmqcliTest);

//
//  We just really need to be sure the connection gets made
// and we do that by sending/receiving a two part msg.
void zmqcliTest::sendrcv_1() {
  uint8_t part1[64];
  uint8_t part2[128];
  for (int i =0; i < sizeof(part1); i++) {
    part1[i] = 2*i;
  }
  for (int i =0; i < sizeof(part2); i++) {
    part2[i] = i;
  }
  iovec v[2];
  v[0].iov_base = part1;
  v[0].iov_len  = sizeof(part1);
  v[1].iov_base = part2;
  v[1].iov_len  = sizeof(part2);
  
  m_pSender->send(v, 2);
  void* rcvData;
  size_t rcvDataLen;
  m_pTestObj->recv(&rcvData, rcvDataLen);
  EQ(sizeof(part1) + sizeof(part2), rcvDataLen);

  EQ(0, memcmp(part1, rcvData, sizeof(part1)));
  uint8_t* p2 = static_cast<uint8_t*>(part1);
  p2 += sizeof(part1);
  EQ(0, memcmp(part2, p2, sizeof(part2)));
  
  free(rcvData);
}

