// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CTestTransport.h"



class testxportTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testxportTest);
  CPPUNIT_TEST(initial_1);
  CPPUNIT_TEST_SUITE_END();


private:
  CTestTransport* m_pTestObject;
public:
  void setUp() {
    m_pTestObject = new CTestTransport;
  }
  void tearDown() {
    delete m_pTestObject;
  }
protected:
  void initial_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testxportTest);

void testxportTest::initial_1() {   // Should be no messages
  
  ASSERT(m_pTestObject->m_sentMessages.empty());
  ASSERT(m_pTestObject->m_messages.empty());
}
