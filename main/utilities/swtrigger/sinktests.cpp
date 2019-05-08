// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CTestTransport.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CDataSinkElement.h"

// We'll do this with test to test data stuff.


class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CTestTransport*  m_pInputData;
  CReceiver*       m_pReceiver;
  CTestTransport*  m_pOutputData;
  CSender*         m_pSender;
  
  CDataSinkElement* m_pTestObj;
  
public:
  void setUp() {
    m_pInputData  = new CTestTransport;
    m_pReceiver   = new CReceiver(*m_pInputData);
    m_pOutputData = new CTestTransport;
    m_pSender     = new CSender(*m_pOutputData);
    
    m_pTestObj    = new CDataSinkElement(*m_pReceiver, *m_pSender);
  }
  void tearDown() {
    delete m_pTestObj;          // sender/receiver deleted but no transports:
    
    delete m_pInputData;
    delete m_pOutputData;
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
