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
#include <zmq.hpp>

static const std::string router("inproc://router");
static const std::string pusher("inproc://pusher");

// A great test of the dispatcher, and a fairly normal use case is for it
// to get data fanned out from a Router and to push it as a fan-in to a
// puller.  We set that up here but as a pipeline only.

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
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
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
