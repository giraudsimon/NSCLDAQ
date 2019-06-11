// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CDataSourceElement.h"
#include "CReceiver.h"
#include "CTestTransport.h"
#include "CZMQRouterTransport.h"
#include "CZMQDealerTransport.h"

#include <string>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <functional>


static const std::string service("inproc://dsource");

// The receiver will need to live in a thread.  This class can be the
// target of an std::thread

class SrcReceiverThread {
public:
  std::vector<std::pair<size_t, void*>> m_received;
  CTransport*   m_pTransport;
  
  SrcReceiverThread(CTransport* pXport) : m_pTransport(pXport) {}
  ~SrcReceiverThread() {
    for (int i =0; i < m_received.size(); i++) {free(m_received[i].second);}
  }
  void operator()() {
    void* pData;
    size_t nBytes;
    do {
      m_pTransport->recv(&pData, nBytes);
      std::pair<size_t, void*> msg(nBytes, pData);
      m_received.push_back(msg);
    } while (nBytes > 0);
  }
};

class dsrcTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(dsrcTest);
  CPPUNIT_TEST(run_1);
  CPPUNIT_TEST(run_2);
  CPPUNIT_TEST_SUITE_END();


private:
  CTestTransport*      m_pTestData;
  CReceiver*           m_pTestReceiver;
  
  CZMQDealerTransport* m_pRecipient;
  CZMQRouterTransport* m_pSender;
  
  CDataSourceElement*  m_pTestObj;
public:
  void setUp() {
    m_pTestData   = new CTestTransport;
    m_pTestReceiver = new CReceiver(*m_pTestData);
    m_pSender     = new CZMQRouterTransport(service.c_str());    
    m_pTestObj    = new CDataSourceElement(*m_pTestReceiver, *m_pSender);
    usleep(1000);
    m_pRecipient  = new CZMQDealerTransport(service.c_str(), 1);
  }
  void tearDown() {
    delete m_pTestObj;   
    delete m_pTestData;
    delete m_pRecipient;
    delete m_pSender;    
    usleep(1000);            // Let the Router run down.
  }
protected:
  void run_1();
  void run_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(dsrcTest);

void dsrcTest::run_1() {
  SrcReceiverThread r(m_pRecipient);
  std::thread t(std::ref(r));
  
  // We need to have at least one message so that the
  // dealer gets registered:
  
  int junk(0);
  m_pTestData->addMessage(&junk, sizeof(junk));
  
  (*m_pTestObj)();
  
  t.join();
  
  // r should have two messages, one with junk, the other empty.
  
  EQ(size_t(2), r.m_received.size());
  EQ(sizeof(junk), r.m_received[0].first);
  int* pData = static_cast<int*>(r.m_received[0].second);
  EQ(junk, *pData);
  EQ(size_t(0),    r.m_received[1].first);
}

void dsrcTest::run_2() {   // try to send a bunch of messages.
  
  SrcReceiverThread r(m_pRecipient);
  std::thread t(std::ref(r));
  
  // Stock the test data source:
  
  int junk;
  for (int i =0; i < 100; i++) {
    junk = i;
    m_pTestData->addMessage(&junk, sizeof(junk));
  }
  
  (*m_pTestObj)();           // Process the messages -> r.
  
  t.join();                  // The thread should exit.
  
  // Check that we have the messages:
  
  EQ(size_t(101), r.m_received.size());
  for (int i =0 ; i < 100; i++) {
    int* pData = static_cast<int*>(r.m_received[i].second);
    EQ(i, *pData);
  }
  EQ(size_t(0), r.m_received[100].first);
  
}