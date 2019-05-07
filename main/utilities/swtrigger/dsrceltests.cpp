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

static const std::string service("inproc://dsource");

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
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
    
    m_pRecipient  = new CZMQDealerTransport(service.c_str(), 1);
    m_pSender     = new CZMQRouterTransport(service.c_str());
    
    m_pTestObj    = new CDataSourceElement(*m_pTestReceiver, *m_pSender);
  }
  void tearDown() {
    delete m_pTestObj;    // deletes testreceiver, and sender.
    delete m_pTestData;
    delete m_pRecipient;
    
    
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
