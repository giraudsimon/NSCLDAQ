

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
#include <CTclControlModule.h>
#undef protected
#undef private

#include <TCLInterpreter.h>
using namespace std;

class CTclControlModuleTests : public CppUnit::TestFixture {
  private:
    unique_ptr<CTclControlModule> m_cmd;
    unique_ptr<CTCLInterpreter>   m_interp;

  public:
    CPPUNIT_TEST_SUITE(CTclControlModuleTests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST(onAttach_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
    m_interp.reset(new CTCLInterpreter);
    m_cmd.reset(new CTclControlModule(*m_interp));
  }
  void tearDown() {
  }
protected:
  void create_0();
  void onAttach_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTclControlModuleTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

void CTclControlModuleTests::create_0 () {
  // already created 
}

void CTclControlModuleTests::onAttach_0 () {

  CControlModule module("name", m_cmd.get());

  CPPUNIT_ASSERT_NO_THROW(module.cget("-ensemble"));
}
