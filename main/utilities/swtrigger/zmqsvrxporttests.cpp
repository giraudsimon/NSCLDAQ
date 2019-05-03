// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CZMQServerTransport.h"
#include "CZMQTransport.h"


#include <string>
#include <zmq.hpp>
#include <unistd.h>

static const std::string service="inproc://zmqservertest";

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
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
    delete m_pRcvSocket;
    delete m_pTestObj;
    
    usleep(100);                     // Let the sockets run down.
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
