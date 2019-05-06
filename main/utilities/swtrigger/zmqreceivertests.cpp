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
#include <zmq.hpp>

static const std::string service("inproc://receivertest");



class Testname : public CppUnit::TestFixture {  // ZMQ Push/Pull.
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
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
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
