// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CTestTransport.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CDataSinkElement.h"

#include <string.h>

// We'll do this with test to test data stuff.


class sinkTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sinkTest);
  CPPUNIT_TEST(atest_1);
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
  void atest_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sinkTest);

void sinkTest::atest_1() {               // Input data gets copied to output
  for (int i =0; i < 100; i++) {
    m_pInputData->addMessage(&i, sizeof(int));  // 100 msgs.
  }
  (*m_pTestObj)();
  
  EQ(size_t(101), m_pOutputData->m_sentMessages.size());
 
  // The first 100 messages are a singlep part message, whose
  // contents form a counting pattern of ints.
  
  for (int i = 0; i < 100; i++) {
    CTestTransport::multipartMessage& mps(m_pOutputData->m_sentMessages[i]);
    EQ(size_t(1), mps.size());
    CTestTransport::message& m(mps[0]);
    EQ(sizeof(int), m.size());
    int d;
    memcpy(&d, m.data(), sizeof(int));
    EQ(i, d);
  }
  // The last message is one part, empty.
  
  EQ(size_t(1), m_pOutputData->m_sentMessages[100].size());
  EQ(size_t(0), m_pOutputData->m_sentMessages[100][0].size());
}
