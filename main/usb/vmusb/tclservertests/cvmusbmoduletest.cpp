

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
#include <CVMUSBModule.h>
#undef protected
#undef private

using namespace std;

class CVMUSBModuleTests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(CVMUSBModuleTests);
    CPPUNIT_TEST(construct_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CVMUSBModuleTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void CVMUSBModuleTests::construct_0() {
  CVMUSBModule bus;
}

