

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iterator>

#include <CMockCCUSB.h>
#include <CControlModule.h>

#define private public
#define protected public
#include <CCCUSBControl.h>
#undef protected
#undef private

using namespace std;

class CCCUSBControlTests : public CppUnit::TestFixture {
  private:
    unique_ptr<CCCUSBControl> m_cmd;

  public:
    CPPUNIT_TEST_SUITE(CCCUSBControlTests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST(set_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
    m_cmd.reset(new CCCUSBControl);
  }
  void tearDown() {
  }
protected:
  void create_0();
  void set_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CCCUSBControlTests);


// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

void CCCUSBControlTests::create_0 () {
  // already created 
}

void CCCUSBControlTests::set_0 () {

  CControlModule module("name", m_cmd.get());

  CMockCCUSB ctlr;
  ctlr.addReturnDatum(1);
  // the next line really just passes a raw list to a CCCUSBReadoutList
  // and executes it. It is actually worth leaving as an standard
  // CCCUSBReadoutList rather than using a logging rdolist.  There are 4 bytes
  // allowed for a return value and a stack with only one entry of 520. The 520
  // corresponds to some control operation.
  module.Set(ctlr,"list","{4} {520}");

  vector<string> expected= {"executeList::begin",
                              "0208",
                              "executeList::end"};
  auto record = ctlr.getOperationsRecord();
  //print_vectors(expected,record);
  CPPUNIT_ASSERT(expected == record);
}
