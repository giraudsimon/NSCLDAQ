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

static const std::string service="inproc://zmqclienttests";



class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CZMQClientTransport*  m_pTestObj;
  CZMQServerTransport*  m_pSender;             // Testsed in zmqsvrxporttests.
public:
  void setUp() {
    
    // ZMQ Claims the order is not important:
    
    m_pTestObj = new CZMQClientTransport(service.c_str(), ZMQ_PULL);
    m_pSender  = new CZMQServerTransport(service.c_str(), ZMQ_PUSH);
  }
  void tearDown() {
    delete m_pTestObj;
    delete m_pSender;
    usleep(100);                     // Let the transport run down.
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
