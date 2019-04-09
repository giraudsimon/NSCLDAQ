// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "HitManager.h"



class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  HitManager* m_pTestObject;
public:
  void setUp() {
    m_pTestObject = new HitManager(10*1.0e9);
  }
  void tearDown() {
    delete m_pTestObject;
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
