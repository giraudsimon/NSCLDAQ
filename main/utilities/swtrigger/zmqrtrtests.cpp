// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include "CZMQRouterTransport.h"
#include "CZMQDealerTransport.h"

#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdint.h>
#include <thread>
#include <vector>
#include <functional>

// Functor we can put in a thread:

class RcvrThread {
public:
  std::vector<std::pair<size_t, void*>> m_msgs;
  CTransport* m_pTransport;

  RcvrThread(CTransport* pTransport): m_pTransport(pTransport)
         {}
  
  void operator()() {
    void* pMsg;
    size_t size(0);
    do {
      m_pTransport->recv(&pMsg, size);
      std::pair<size_t, void*> msg;
      msg.first = size;
      msg.second = pMsg;
      m_msgs.push_back(msg);
    } while (size != 0);
  }
};

static const std::string service("inproc://routertest");

class routerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(routerTest);
  CPPUNIT_TEST(sndrcv_1);
  CPPUNIT_TEST(sndrcv_2);
  CPPUNIT_TEST(sndrcv_2);
  
  CPPUNIT_TEST(end_1);
  CPPUNIT_TEST_SUITE_END();


private:
  CZMQRouterTransport* m_pTestObj;
  CZMQDealerTransport* m_pReceiver1;
  CZMQDealerTransport* m_pReceiver2;
  
public:
  void setUp() {
    m_pTestObj = new CZMQRouterTransport(service.c_str());
    m_pReceiver1 = new CZMQDealerTransport(service.c_str(), 1);
    m_pReceiver2 = new CZMQDealerTransport(service.c_str(), 2);
  }
  void tearDown() {
    delete m_pReceiver1;
    delete m_pReceiver2;
    delete m_pTestObj;
    usleep(1000);
  }
protected:
  void sndrcv_1();
  void sndrcv_2();
  void sndrcv_3();
  
  void end_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(routerTest);

void routerTest::sndrcv_1() {             // Single part message, one dealer.
  RcvrThread r(m_pReceiver1);
  std::thread t(std::ref(r));                  // Makes a request for data.
  
  uint8_t  msg[128];
  for (int i = 0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  iovec v;
  v.iov_base =msg;
  v.iov_len = sizeof(msg);
  m_pTestObj->send(&v, 1);          // Send a message.
  
  // Make an end msg:
  
  v.iov_len = 0;
  m_pTestObj->send(&v, 1);
  
  t.join();                       // Second msg should have cause t to exit.
  
  // Should have received two messages.. the first with
  // msg, the second with an empty msg.
  
  EQ(size_t(2), r.m_msgs.size());
  EQ(sizeof(msg), r.m_msgs[0].first);
  EQ(0, memcmp(msg, r.m_msgs[0].second, sizeof(msg)));
  EQ(size_t(0), r.m_msgs[1].first);
  
  for (int i =0; i < r.m_msgs.size(); i++) {
    free(r.m_msgs[i].second);
  }
}
void routerTest::sndrcv_2()             // multipart messages
{
  RcvrThread r1(m_pReceiver1);
  RcvrThread r2(m_pReceiver2);
  
  std::thread t1(std::ref(r1));
  usleep(100);                      // wait for the request to happen.
  std::thread t2(std::ref(r2));
  
  uint8_t msg1[128];
  uint8_t msg2[64];
  
  for(int i = 0; i < sizeof(msg1); i++) {
    msg1[i] = i;
  }
  for (int i = 0; i < sizeof(msg2); i++) {
    msg2[i] = sizeof(msg2) - i;
  }
  
  iovec v;
  v.iov_base=msg1;
  v.iov_len =sizeof(msg1);
  
  m_pTestObj->send(&v, 1);            // ->thread 1.
  
  v.iov_base = msg2;
  v.iov_len  = sizeof(msg2);
  m_pTestObj->send(&v, 1);           // -> thread 2.
  
  v.iov_len = 0;
  m_pTestObj->send(&v, 1);           // get the threads to 
  m_pTestObj->send(&v, 1);           // exit.
  
  t1.join();                         // wait for the threads.
  t2.join();                         // to exit.
  
  // Check the data received by the threads:
  
  EQ(size_t(2), r1.m_msgs.size());
  EQ(sizeof(msg1), r1.m_msgs[0].first);
  EQ(0, memcmp(msg1, r1.m_msgs[0].second, sizeof(msg1)));
  EQ(size_t(0), r1.m_msgs[1].first);
  
  EQ(size_t(2), r2.m_msgs.size());
  EQ(sizeof(msg2), r2.m_msgs[0].first);
  EQ(0, memcmp(msg2, r2.m_msgs[0].second, sizeof(msg2)));
  EQ(size_t(0), r2.m_msgs[1].first);
  
}
void routerTest::sndrcv_3()
{
  RcvrThread r1(m_pReceiver1);
  RcvrThread r2(m_pReceiver2);
  
  std::thread t1(r1);
  usleep(100);                     // SO r1 can request data.
  std::thread t2(r2);              // r1, then r2 requesting.
  
  uint8_t msg1[128];
  uint8_t msg2[64];
  
  for(int i = 0; i < sizeof(msg1); i++) {
    msg1[i] = i;
  }
  for (int i = 0; i < sizeof(msg2); i++) {
    msg2[i] = sizeof(msg2) - i;
  }
  
  iovec v1[2];
  v1[0].iov_base = msg1;
  v1[0].iov_len  = sizeof(msg1);
  v1[1].iov_base = msg2;
  v1[1].iov_len  = sizeof(msg2);
  
  iovec v2[2];
  v2[0].iov_base = msg2;
  v2[0].iov_len  = sizeof(msg2);
  v2[1].iov_base = msg1;
  v2[1].iov_len  = sizeof(msg1);
  
  m_pTestObj->send(v1, 2);                 // Send to t1
  m_pTestObj->send(v2, 2);                 // Send to t2.
  
  v1[0].iov_len = 0;
  m_pTestObj->send(v1, 1);                // end to t1 or t2
  m_pTestObj->send(v1, 1);                // end to t2 or t1 whichever is left.
  
  t1.join();
  t2.join();
  
  // t1 should have m1 then m2 in one message.
  
  EQ(size_t(2), r1.m_msgs.size());
  EQ(sizeof(msg1) + sizeof(msg2), r1.m_msgs[0].first);
  EQ(0, memcmp(msg1, r1.m_msgs[0].second, sizeof(msg1)));  // first part is msg1.
  uint8_t* p2 = static_cast<uint8_t*>(r1.m_msgs[0].second);
  p2 += sizeof(msg1);
  EQ(0, memcmp(msg2, p2, sizeof(msg2)));
  
  // t2 gets them in opposite order.
  
  EQ(size_t(2), r2.m_msgs.size());
  EQ(sizeof(msg1) + sizeof(msg2), r2.m_msgs[0].first);
  EQ(0, memcmp(msg2, r2.m_msgs[0].second, sizeof(msg2)));
  p2 = static_cast<uint8_t*>(r2.m_msgs[0].second);
  p2 += sizeof(msg2);
  EQ(0, memcmp(msg1, p2, sizeof(msg1)));
  
}

void routerTest::end_1()     // End sends no payload mesg.
{
  RcvrThread r(m_pReceiver1);
  std::thread t(std::ref(r));
  usleep(1000);              // Let it get a request in first.
  
  // need to send it something to let end work.
  
  uint8_t msg = 1;
  iovec v;
  v.iov_base  = &msg;
  v.iov_len   = sizeof(msg);
  m_pTestObj->send(&v, 1);     /// Now the thread is a registered client.
  
  m_pTestObj->end();
  t.join();                    // should exit.
  
  // Thred got a single byte message and a null mesg.
  
  EQ(size_t(2), r.m_msgs.size());
  EQ(size_t(1), r.m_msgs[0].first);
  EQ(size_t(0), r.m_msgs[1].first);
}