

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <CControlModule.h>

#define private public
#define protected public
#include <CV812.h>
#undef protected
#undef private

using namespace std;

class CV812Tests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(CV812Tests);
    CPPUNIT_TEST(construct_0);
    CPPUNIT_TEST(cget_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct_0();
  void cget_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CV812Tests);

void CV812Tests::construct_0() {
  CV812 bus;
}

void CV812Tests::cget_0() {
  unique_ptr<CV812> bus(new CV812);

  CControlModule module("test",move(bus));

  CPPUNIT_ASSERT_NO_THROW( module.cget("-base"));
  CPPUNIT_ASSERT_NO_THROW( module.cget("-file"));
}


