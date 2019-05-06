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

std::string service("inproc://sendertests");


class sendertest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sendertest);
  CPPUNIT_TEST(aTest);
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
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sendertest);

void sendertest::aTest() {
}
