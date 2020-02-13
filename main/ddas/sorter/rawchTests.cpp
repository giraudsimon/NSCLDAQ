// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"



#define private public
#include "RawChannel.h"
#undef private

#include <stdexcept>

class rawchTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rawchTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {

  }
  void tearDown() {

  }
protected:
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(rawchTest);
