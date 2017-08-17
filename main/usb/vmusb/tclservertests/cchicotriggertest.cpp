

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iterator>
#include <CControlModule.h>
#include <CMockVMUSB.h>

#define private public
#define protected public
#include <ChicoTrigger.h>
#undef protected
#undef private

using namespace std;

class ChicoTriggerTests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(ChicoTriggerTests);
    CPPUNIT_TEST(construct_0);
    CPPUNIT_TEST(update_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct_0();
  void update_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(ChicoTriggerTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void ChicoTriggerTests::construct_0() {
  ChicoTrigger bus;
}

void ChicoTriggerTests::update_0() {
  std::unique_ptr<CControlHardware> hdwr(new ChicoTrigger);
  CControlModule module("name", hdwr.get());
  module.configure("-base","0xffff0000");

  CPPUNIT_ASSERT( 0xffff0000 == module.getUnsignedParameter("-base") );
  
}


