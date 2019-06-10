

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iterator>

#include <CControlModule.h>
#include <CMockCCUSB.h>

#define private public
#define protected public
#include <CPH7106.h>
#undef protected
#undef private

using namespace std;

class CPH7106Tests : public CppUnit::TestFixture {
  private:
    unique_ptr<CPH7106> m_cmd;

  public:
    CPPUNIT_TEST_SUITE(CPH7106Tests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST(onAttach_0);
    CPPUNIT_TEST(initialize_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
    m_cmd.reset(new CPH7106);
  }
  void tearDown() {
  }
protected:
  void create_0();
  void onAttach_0();
  void initialize_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CPH7106Tests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

void CPH7106Tests::create_0 () {
  // already created 
}

void CPH7106Tests::onAttach_0 () {

  CControlModule module("name", m_cmd.get());

  CPPUNIT_ASSERT_NO_THROW(module.cget("-slot"));
}

// This should be sufficinet to test the remainder of all the methods with
// respect to the functionality of the CControlHardware and CControlModule. The
// rest of the operations only really depend on the getSlot method that is
// tested indirectly with the initialize_0
void CPH7106Tests::initialize_0 () 
{
  CControlModule module("name", m_cmd.get());
  module.configure("-slot", "12");

  CMockCCUSB ctlr;
  module.Initialize(ctlr);

  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "control 12 0 26";
  expected[2] = "executeList::end";

  auto record = ctlr.getOperationsRecord();
  //print_vectors(expected, record);
  CPPUNIT_ASSERT(expected == record);
    
}
