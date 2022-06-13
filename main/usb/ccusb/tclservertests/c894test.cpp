

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <CControlModule.h>
#include <CMockCCUSB.h>

#include <iterator>
#include <iostream>

#define private public
#define protected public
#include <C894.h>
#undef protected
#undef private

using namespace std;

class C894Tests : public CppUnit::TestFixture {
  private:
    unique_ptr<C894> m_cmd;

  public:
    CPPUNIT_TEST_SUITE(C894Tests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST(onAttach_0);
//    CPPUNIT_TEST(initialize_0);
    CPPUNIT_TEST(update_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
    m_cmd.reset(new C894);
  }
  void tearDown() {
  }
protected:
  void create_0();
  void onAttach_0();
  void initialize_0();
  void update_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(C894Tests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void C894Tests::create_0 () {
  // already created 
}

void C894Tests::onAttach_0 () {
  CControlModule module("test", m_cmd.get());

  CPPUNIT_ASSERT_NO_THROW( module.cget("-slot") );
  CPPUNIT_ASSERT_NO_THROW( module.cget("-file") );
}

/*
 * Non-trivial because the initialize method loads settings from a file... 
 * But... we can test a good portion of the functionality through the Update
 * method
 */
void C894Tests::initialize_0 () {
  CControlModule module("test", m_cmd.get());
  module.configure("-slot","10");

  CMockCCUSB ctlr;
  module.Initialize(ctlr);

  std::vector<std::string> expected(3);
  expected[0] = "executeList::begin"; 
  expected[1] = "control 10 0 26";
  expected[2] = "executeList::end"; 

  auto record = ctlr.getOperationsRecord();

  //print_vectors(expected,record);
  CPPUNIT_ASSERT(expected == record);
}

void C894Tests::update_0 () {
  for (size_t index=0; index<16; ++index) {
    m_cmd->m_thresholds[index] = -1*index;
  }

  m_cmd->m_widths[0] = 1;
  m_cmd->m_widths[1] = 2;

  m_cmd->m_inhibits = 10;
  m_cmd->m_majority = 1; // --> 5

  CControlModule module("test", m_cmd.get());
  module.configure("-slot","10");

  CMockCCUSB ctlr;
  module.Update(ctlr);

  std::vector<std::string> expected(22);
  expected[0] = "executeList::begin";
  expected[1] = "write16 10 0 16 0";
  expected[2] = "write16 10 1 16 1";
  expected[3] = "write16 10 2 16 2";
  expected[4] = "write16 10 3 16 3";
  expected[5] = "write16 10 4 16 4";
  expected[6] = "write16 10 5 16 5";
  expected[7] = "write16 10 6 16 6";
  expected[8] = "write16 10 7 16 7";
  expected[9] = "write16 10 8 16 8";
  expected[10] = "write16 10 9 16 9";
  expected[11] = "write16 10 10 16 10";
  expected[12] = "write16 10 11 16 11";
  expected[13] = "write16 10 12 16 12";
  expected[14] = "write16 10 13 16 13";
  expected[15] = "write16 10 14 16 14";
  expected[16] = "write16 10 15 16 15";
  expected[17] = "write16 10 0 18 1";
  expected[18] = "write16 10 1 18 2";
  expected[19] = "write16 10 0 17 10";
  expected[20] = "write16 10 0 20 6";
  expected[21] = "executeList::end";

  auto record = ctlr.getOperationsRecord();

  //print_vectors(expected,record);
  CPPUNIT_ASSERT(expected == record);
}
