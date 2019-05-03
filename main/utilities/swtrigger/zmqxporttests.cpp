// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "Asserts.h"
#include "CZMQTransport.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>



static const char* zmquri="inproc://czmqtransporttest";

/*
 We'll set up raw push/pull sockets and use them to test
 CZMQTransports that are wrapped around them using CZMQRawTransport.
 Push servers, Pull connects.
 Note the singleton natore of the zmq context makes this not _quite_
 idempotent.
*/
class zmqxporttest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqxporttest);
  CPPUNIT_TEST(recv_1);
  CPPUNIT_TEST(recv_2);
  
  CPPUNIT_TEST(send_1);
  CPPUNIT_TEST(send_2);
  
  CPPUNIT_TEST(sndrcv_1);
  CPPUNIT_TEST(sndrcv_2);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::socket_t*  m_pPushSock;
  zmq::socket_t*  m_pPullSock;
  
  CZMQRawTransport* m_pPushTransport;
  CZMQRawTransport* m_pPullTransport;
public:
  void setUp() {
    zmq::context_t* pContext = CZMQTransport::getContext();
    
    m_pPushSock = new zmq::socket_t(*pContext, ZMQ_PUSH);
    m_pPushSock->bind(zmquri);
    m_pPushTransport = new CZMQRawTransport(m_pPushSock);
    
    m_pPullSock = new zmq::socket_t(*pContext, ZMQ_PULL);
    m_pPullSock->connect(zmquri);
    m_pPullTransport = new CZMQRawTransport(m_pPullSock);
  }
  void tearDown() {
    
    delete m_pPushTransport;             // deletes the socket.
    delete m_pPullTransport;
    usleep(500);                       // Let the transport run down?
  }
protected:
  void recv_1();
  void recv_2();
  
  void send_1();
  void send_2();
  
  void sndrcv_1();
  void sndrcv_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zmqxporttest);

void zmqxporttest::recv_1() {   // Receive single part message:
  
  uint8_t msg[256];
  for (int i =0; i < 256; i++) {
    msg[i] = i;
  }
  zmq::message_t zmqMsg(sizeof(msg));
  memcpy(zmqMsg.data(), msg, sizeof(msg));
  m_pPushSock->send(zmqMsg, 0);
  
  // RECV on the pull transport should get me the message back:
  
  void* pMsg;
  size_t nBytes;
  m_pPullTransport->recv(&pMsg, nBytes);
  EQ(sizeof(msg), nBytes);
  ASSERT(memcmp(msg, pMsg, sizeof(msg)) == 0);
  free(pMsg);
}

void zmqxporttest::recv_2()    // Message parts get concatenated.
{
  uint8_t msg1[256];
  for (int i = 0; i < 256; i ++) {
    msg1[i] = i;
  }
  uint8_t msg2[128];
  for (int i =0; i < 128; i++) {
    msg2[i] = 127-i;
  }
  zmq::message_t zmqMsg1(sizeof(msg1));
  zmq::message_t zmqMsg2(sizeof(msg2));
  memcpy(zmqMsg1.data(), msg1, sizeof(msg1));
  memcpy(zmqMsg2.data(), msg2, sizeof(msg2));
  
  m_pPushSock->send(zmqMsg1, ZMQ_SNDMORE);
  m_pPushSock->send(zmqMsg2, 0);
  
  uint8_t* pMsg;
  size_t nBytes;
  m_pPullTransport->recv(reinterpret_cast<void**>(&pMsg), nBytes);
  EQ(sizeof(msg1) + sizeof(msg2), nBytes);
  
  ASSERT(memcmp(msg1, pMsg, sizeof(msg1)) == 0);
  ASSERT(memcmp(msg2, pMsg + sizeof(msg1), sizeof(msg2)) == 0);
  
}
void zmqxporttest::send_1()       // Send single part message.
{
  uint8_t msg[256];
  for (int i = 0; i < 256; i++) {
    msg[i] = i;
  }
  iovec v;
  
  v.iov_base = msg;
  v.iov_len  = sizeof(msg);
  
  m_pPushTransport->send(&v, 1);              // Send the message.
  
  zmq::message_t zmqMsg;
  m_pPullSock->recv(&zmqMsg, 0);
  EQ(sizeof(msg), zmqMsg.size());
  ASSERT(memcmp(msg, zmqMsg.data(), sizeof(msg)) == 0);
}
void zmqxporttest::send_2()
{
  uint8_t msg1[256];
  for (int i = 0; i < 256; i ++) {
    msg1[i] = i;
  }
  uint8_t msg2[128];
  for (int i =0; i < 128; i++) {
    msg2[i] = 127-i;
  }
  iovec v[2];
  v[0].iov_base = msg1;
  v[0].iov_len  = sizeof(msg1);
  v[1].iov_base = msg2;
  v[1].iov_len  = sizeof(msg2);
  
  m_pPushTransport->send(v, 2);       // Should send 2x message parts.
  
  zmq::message_t part1;
  zmq::message_t part2;
  
  m_pPullSock->recv(&part1, 0);
  
  uint64_t more(0);
  size_t   s(sizeof(more));
  
  m_pPullSock->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(more);
  
  m_pPullSock->recv(&part2, 0);
  
  m_pPullSock->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(!more);
  
  EQ(sizeof(msg1), part1.size());
  EQ(sizeof(msg2), part2.size());
  
  ASSERT(memcmp(msg1, part1.data(), sizeof(msg1)) == 0);
  ASSERT(memcmp(msg2, part2.data(), sizeof(msg2)) == 0);
}
void zmqxporttest::sndrcv_1()   // Single message segment send/rcv.
{
  uint8_t msg[256];
  for (int i =0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  iovec v;
  v.iov_base = msg;
  v.iov_len  = sizeof(msg);
  
  m_pPushTransport->send(&v, 1);         // Single part.
  
  void* pData;
  size_t   nBytes;
  
  m_pPullTransport->recv((&pData), nBytes);
  EQ(sizeof(msg), nBytes);
  ASSERT(memcmp(msg, pData, sizeof(msg)) == 0);
}
void zmqxporttest::sndrcv_2()        // 2 part message.
{
  uint8_t msg1[256];
  for (int i = 0; i < 256; i ++) {
    msg1[i] = i;
  }
  uint8_t msg2[128];
  for (int i =0; i < 128; i++) {
    msg2[i] = 127-i;
  }
  iovec v[2];
  v[0].iov_base = msg1;
  v[0].iov_len  = sizeof(msg1);
  v[1].iov_base = msg2;
  v[1].iov_len  = sizeof(msg2);
  
  m_pPushTransport->send(v, 2);       // Should send 2x message parts.
  
  uint8_t* pData;
  size_t   nBytes;
  m_pPullTransport->recv(reinterpret_cast<void**>(&pData), nBytes);
  EQ(sizeof(msg1) + sizeof(msg2), nBytes);
  
  ASSERT(memcmp(msg1, pData, sizeof(msg1)) == 0);
  ASSERT(memcmp(msg2, pData + sizeof(msg1), sizeof(msg2)) == 0);
}