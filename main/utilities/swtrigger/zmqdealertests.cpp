// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CZMQDealerTransport.h"
#include "CZMQServerTransport.h"

#include <zmq.hpp>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <vector>
static const std::string service="inproc://dealertest";

class DealerThread 
{
public:
  std::vector<std::pair<size_t, void*>>& m_ReceivedMsgs;
  
public:
  DealerThread(std::vector<std::pair<size_t, void*>>& ReceivedMsgs) :
    m_ReceivedMsgs(ReceivedMsgs) {}
  void operator()(CZMQDealerTransport* xport);
  
};

// Receive messages until one with no size.
void
DealerThread::operator()(CZMQDealerTransport* xport)
{
  void* pMsg;
  size_t msgSize;
  
  do {
    
    xport->recv(&pMsg, msgSize); 
    m_ReceivedMsgs.push_back({msgSize, pMsg});
    
  } while (msgSize > 0);
  
}


class zmqdealertest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqdealertest);
  CPPUNIT_TEST(rcv_1);
  CPPUNIT_TEST(rcv_2);
  CPPUNIT_TEST(rcv_3);             // Multiple receivers.
  CPPUNIT_TEST_SUITE_END();


private:
  CZMQDealerTransport* m_pTestObj1;
  CZMQDealerTransport* m_pTestObj2;
  
  CZMQServerTransport* m_pRouter;
public:
  void setUp() {
    m_pTestObj1 = new CZMQDealerTransport(service.c_str(), 1);
    m_pTestObj2 = new CZMQDealerTransport(service.c_str(), 2);
    
    m_pRouter = new CZMQServerTransport(service.c_str(), ZMQ_ROUTER);
  }
  void tearDown() {
    delete m_pTestObj1;
    delete m_pTestObj2;
    delete m_pRouter;
    
    usleep(100);
  }
protected:
  void rcv_1();
  void rcv_2();
  void rcv_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zmqdealertest);

void zmqdealertest::rcv_1() {
  std::vector<std::pair<size_t, void*>> ReceivedMsgs;
  DealerThread t(ReceivedMsgs);
  std::thread thr(t, m_pTestObj1);
  
  // The Router needs to receive the message request - it should contain the
  // uint32_t id of the requestor.
  
  void* requestor;
  size_t reqSize;
  m_pRouter->recv(&requestor, reqSize);
  
  
  // As a router we need to send a three part message:
  // - Identity
  // - Delimeter (empty part).
  // - body.
  //   ZMQ will strip off the identity
  //   CZMQDealerTransport will strip off the delimeter.
  //   So the dealer thread will just see the body.
  //   which will be empty to make it exit:
  
  iovec parts[3];
  parts[0].iov_base = requestor;
  parts[0].iov_len  = reqSize;
  
  parts[1].iov_base = nullptr;
  parts[1].iov_len  = 0;
  
  parts[2].iov_base = nullptr;
  parts[2].iov_len  = 0;
  m_pRouter->send(parts, 3);
  
  
  thr.join();                
  free(requestor);
  
  
  /// Check the dealer got the messages properly:
  
  EQ(size_t(1), ReceivedMsgs.size());
  EQ(size_t(0), ReceivedMsgs[0].first);
  
  // Free the dealer messagse:
  
  for (int  i =0; i < ReceivedMsgs.size(); i++) {
    free(ReceivedMsgs[i].second);
  }
  
}

//
//  Send an actual message and then an end message to a single thread:
//
void zmqdealertest::rcv_2()
{
  std::vector<std::pair<size_t, void*>> ReceivedMsgs;
  DealerThread t(ReceivedMsgs);
  std::thread thr(t, m_pTestObj1);
  
  // The Router needs to receive the message request - it should contain the
  // uint32_t id of the requestor.
  
  void* requestor;
  size_t reqSize;
  m_pRouter->recv(&requestor, reqSize);
  for(int i =0; i < ReceivedMsgs.size(); i++) {
    free(ReceivedMsgs[i].second);
  }
  
  // As a router we need to send a three part message:
  // - Identity
  // - Delimeter (empty part).
  // - body.
  //   ZMQ will strip off the identity
  //   CZMQDealerTransport will strip off the delimeter.
  //   So the dealer thread will just see the body.
  //   which will be empty to make it exit:
  uint8_t msg[100];
  for (int i =0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  
  iovec parts[3];
  parts[0].iov_base = requestor;
  parts[0].iov_len  = reqSize;
  
  parts[1].iov_base = nullptr;
  parts[1].iov_len  = 0;
  
  parts[2].iov_base = msg;
  parts[2].iov_len  = sizeof(msg);;
  
  m_pRouter->send(parts, 3);
  

  
  // Now the EOF message (note we need to receive the reqeust):
  
  free(requestor);
  m_pRouter->recv(&requestor, reqSize);
  
  parts[2].iov_len = 0;             // EOF msg.
  m_pRouter->send(parts,3);
  
  thr.join();                      // That should maake thread exit.
  
  EQ(size_t(2), ReceivedMsgs.size());
  EQ(sizeof(msg), ReceivedMsgs[0].first);
  EQ(0, memcmp(msg, ReceivedMsgs[0].second, sizeof(msg)));
  EQ(size_t(0), ReceivedMsgs[1].first);
  
  free(requestor);
  for(int i =0; i < ReceivedMsgs.size(); i++) {
    free(ReceivedMsgs[i].second);
  }
  
}

void zmqdealertest::rcv_3()
{
  std::vector<std::pair<size_t, void*>> ReceivedMsgs1;
  DealerThread t1(ReceivedMsgs1);
  std::thread thr1(t1, m_pTestObj1);
  
  usleep(100);     // Let the thread get its request for data in.
  
  std::vector<std::pair<size_t, void*>> ReceivedMsgs2;
  DealerThread t2(ReceivedMsgs2);
  std::thread thr2(t2, m_pTestObj2);
  
  usleep(100);    // Let him get is req in too.
  
  // If we do the recv it should:
  // 1. Come from id 1.
  // 2. If we respond to it with an EOF msg, that thread should be joinable.
  //
  
  void* data;
  size_t idsize;
  m_pRouter->recv(&data, idsize);
  uint64_t* pId = static_cast<uint64_t*>(data);
  EQ(uint64_t(1), *pId);
  
  iovec msgParts[3];
  msgParts[0].iov_base = data;                // Destination.
  msgParts[0].iov_len = idsize;
  msgParts[1].iov_base = nullptr;             // Delimieter.
  msgParts[1].iov_len  = 0;
  msgParts[2].iov_base = nullptr;             // Empty Payload.
  msgParts[2].iov_len  = 0;
  m_pRouter->send(msgParts, 3);
  thr1.join();                              // should complete!!
  EQ(size_t(1), ReceivedMsgs1.size());
  EQ(size_t(0), ReceivedMsgs1[0].first);
  free(data);
  
  // This should come from id 2:
  
  m_pRouter->recv(&data, idsize);
  pId = static_cast<uint64_t*>(data);
  EQ(uint64_t(2), *pId);
  m_pRouter->send(msgParts, 3);
  thr2.join();                              /// this is now joinable too.
  free(data);
  EQ(size_t(1), ReceivedMsgs2.size());
  EQ(size_t(0), ReceivedMsgs2[0].first);
  
  free(ReceivedMsgs1[0].second);
  free(ReceivedMsgs2[0].second);
  
}