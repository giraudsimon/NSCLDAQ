

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
#include <CV6533.h>
#undef protected
#undef private

using namespace std;

class CV6533Tests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(CV6533Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(CV6533Tests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void CV6533Tests::construct_0() {
  CV6533 bus;
}

void CV6533Tests::update_0() {

  unique_ptr<CV6533> hdwr(new CV6533);
  CControlModule module("name", hdwr.get());
  module.configure("-base","0xffff0000");

  CMockVMUSB ctlr;
  module.Initialize(ctlr);

  vector<string> expected;
  expected.push_back("executeList::begin");
  expected.push_back("addWrite16 ffff0090 0d 0");
  expected.push_back("addWrite16 ffff0080 0d 0");
  expected.push_back("addWrite16 ffff0110 0d 0");
  expected.push_back("addWrite16 ffff0100 0d 0");
  expected.push_back("addWrite16 ffff0190 0d 0");
  expected.push_back("addWrite16 ffff0180 0d 0");
  expected.push_back("addWrite16 ffff0210 0d 0");
  expected.push_back("addWrite16 ffff0200 0d 0");
  expected.push_back("addWrite16 ffff0290 0d 0");
  expected.push_back("addWrite16 ffff0280 0d 0");
  expected.push_back("addWrite16 ffff0310 0d 0");
  expected.push_back("addWrite16 ffff0300 0d 0");
  expected.push_back("executeList::end");

  auto record = ctlr.getOperationRecord();
//  print_vectors(expected,record);

  CPPUNIT_ASSERT( expected == record );
  
}


