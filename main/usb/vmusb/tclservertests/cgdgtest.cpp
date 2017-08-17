

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
#include <CGDG.h>
#undef protected
#undef private

using namespace std;

class CGDGTests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(CGDGTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(CGDGTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void CGDGTests::construct_0() {
  CGDG bus;
}

void CGDGTests::update_0() {

  unique_ptr<CGDG> hdwr(new CGDG);
  CControlModule module("name", hdwr.get());
  module.configure("-base","0xffff0000");

  CMockVMUSB ctlr;
  module.Update(ctlr);

  vector<string> expected;
  expected.push_back("executeList::begin");
  expected.push_back("addWrite32 ffff0084 39 286331153"); 
  expected.push_back("addWrite32 ffff008c 39 50462976"); 
  expected.push_back("addWrite32 ffff0090 39 117835012"); 
  expected.push_back("addWrite32 ffff0094 39 50462976"); 
  expected.push_back("addWrite32 ffff0098 39 117835012"); 

  expected.push_back("addRead32 ffff0040 39"); 
  expected.push_back("addRead32 ffff0044 39"); 
  expected.push_back("addRead32 ffff0048 39"); 
  expected.push_back("addRead32 ffff004c 39"); 
  expected.push_back("addRead32 ffff0050 39"); 
  expected.push_back("addRead32 ffff0054 39"); 
  expected.push_back("addRead32 ffff0058 39"); 
  expected.push_back("addRead32 ffff005c 39"); 
  expected.push_back("addRead32 ffff0060 39"); 
  expected.push_back("addRead32 ffff0064 39"); 
  expected.push_back("addRead32 ffff0068 39"); 
  expected.push_back("addRead32 ffff006c 39"); 
  expected.push_back("addRead32 ffff0070 39"); 
  expected.push_back("addRead32 ffff0074 39"); 
  expected.push_back("addRead32 ffff0078 39"); 
  expected.push_back("addRead32 ffff007c 39"); 

  expected.push_back("executeList::end");

  auto record = ctlr.getOperationRecord();
//  print_vectors(expected,record);
  CPPUNIT_ASSERT( expected == record );
  
}


