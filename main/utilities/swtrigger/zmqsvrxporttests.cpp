// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CZMQServerTransport.h"
#include "CZMQTransport.h"


#include <string>
#include <zmq.hpp>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

static const std::string service="inproc://zmqservertest";

class zmqsvrxportTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqsvrxportTest);
  CPPUNIT_TEST(sendRcv_1);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::socket_t*           m_pRcvSocket;
  CZMQServerTransport*     m_pTestObj;
  CZMQRawTransport*        m_pReceiver;
  
public:
  void setUp() {
    m_pTestObj = new CZMQServerTransport(service.c_str(), ZMQ_PUSH);
    
    m_pRcvSocket = new zmq::socket_t(*CZMQTransport::getContext(), ZMQ_PULL);
    m_pRcvSocket->connect(service.c_str());
    m_pReceiver = new CZMQRawTransport(m_pRcvSocket);
  }
  void tearDown() {
    delete m_pReceiver;
    delete m_pTestObj;
    
    usleep(100);                     // Let the sockets run down.
  }
protected:
  void sendRcv_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zmqsvrxportTest);

// Really we just need to know there's connectivity because
// Send/recv are tested elsewhere  we test this just by sending
// a simple multipart message
//
void zmqsvrxportTest::sendRcv_1() {   
  uint8_t part1[128];
  uint8_t part2[64];
  for (int i =0; i < sizeof(part1); i++) {
    part1[i] = i;
  }
  for (int i =0; i < sizeof(part2); i++) {
    part2[i] = 2*i;
  }
  iovec v[2];
  v[0].iov_base = part1;
  v[0].iov_len  = sizeof(part1);
  v[1].iov_base = part2;
  v[1].iov_len  = sizeof(part2);
  
  m_pTestObj->send(v, 2);
  
  void* pReceivedData;
  size_t nBytes;
  m_pReceiver->recv(&pReceivedData, nBytes);
  
  EQ(sizeof(part1) + sizeof(part2), nBytes);
  EQ(0, memcmp(part1, pReceivedData, sizeof(part1)));
  uint8_t* p2 = static_cast<uint8_t*>(pReceivedData);
  p2 += sizeof(part1);
  EQ(0, memcmp(part2, p2, sizeof(part2)));
}
