// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include "CDispatcher.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CZMQRouterTransport.h"
#include "CZMQDealerTransport.h"
#include "CZMQServerTransport.h"           // PUSH
#include "CZMQClientTransport.h"           // PULL.

#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <stdint.h>
#include <sys/uio.h>
#include <zmq.hpp>

static const std::string router("inproc://router");
static const std::string pusher("inproc://pusher");

//  The dispatcher needs to run in a thread, here's a functor for that thread:

class DispatThread {
private:
  CDispatcher*  m_pDispat;
public:
  DispatThread(CDispatcher* pDispat) : m_pDispat(pDispat) {}
  void operator() () {
    size_t size;
    void*  pData;
    do {
      m_pDispat->receiveWorkItem(&pData, size);
      m_pDispat->sendWorkItem(pData, size);
      free(pData);
    } while (size > 0);
  }
  
};

// A great test of the dispatcher, and a fairly normal use case is for it
// to get data fanned out from a Router and to push it as a fan-in to a
// puller.  We set that up here but as a pipeline only.
class dispatTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(dispatTest);
  CPPUNIT_TEST(dispat_1);
  CPPUNIT_TEST_SUITE_END();


private:
  CZMQRouterTransport*  m_pRouter;
  CZMQDealerTransport*  m_pDealer;
  CZMQServerTransport*  m_pPusher;
  CZMQClientTransport*  m_pPuller;
  
  CSender*              m_pPushSender;
  CReceiver*            m_pDealerReceiver;
  
  CDispatcher*          m_pTestObj;      
public:
  void setUp() {
    m_pRouter = new CZMQRouterTransport(router.c_str());
    m_pDealer = new CZMQDealerTransport(router.c_str(), 1);
    m_pPusher = new CZMQServerTransport(pusher.c_str(), ZMQ_PUSH);
    m_pPuller = new CZMQClientTransport(pusher.c_str(), ZMQ_PULL);
    
    m_pPushSender = new CSender(*m_pPusher);
    m_pDealerReceiver = new CReceiver(*m_pDealer);
    
    m_pTestObj = new CDispatcher(m_pDealerReceiver, m_pPushSender);
  }
  void tearDown() {
    delete m_pTestObj;            // Deletes the senders, not the xports.
    delete m_pRouter;
    delete m_pPuller;        
    
    delete m_pPusher;
    delete m_pDealer;
    
  }
protected:
  void dispat_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(dispatTest);

void dispatTest::dispat_1() {
  DispatThread d(m_pTestObj);
  std::thread t(std::ref(d));
  
  // Have the router send data to the dispatcher... pulller should get it.
  
  uint8_t msg[128];
  for (int i =0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  iovec m;
  m.iov_base =  msg;
  m.iov_len  = sizeof(msg);
  
  m_pRouter->send(&m, 1);
  
  void *pData;
  size_t nBytes;
  m_pPuller->recv(&pData, nBytes);
  EQ(sizeof(msg), nBytes);
  EQ(0, memcmp(msg, pData, sizeof(msg)));
  free(pData);
  
  // Send the end to get the thread to exit -- puller also should get it:
  
  m_pRouter->end();
  m_pPuller->recv(&pData, nBytes);
  EQ(size_t(0), nBytes);
  
  
  t.join();                        // Join the exited thread.
  
}

