

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iterator>
#include <CControlModule.h>
#include <CMockVMUSB.h>
#include <CLoggingReadoutList.h>

#define private public
#define protected public
#include <CMDGG16Control.h>
#undef protected
#undef private

using namespace std;

class CMDGG16ControlTests : public CppUnit::TestFixture {
  private:
    CControlModule* m_pMod;

  public:
    CPPUNIT_TEST_SUITE(CMDGG16ControlTests);
    CPPUNIT_TEST(onAttach_0);

    CPPUNIT_TEST(initialize_0);
    CPPUNIT_TEST(initialize_1);

    CPPUNIT_TEST(set_0);
    CPPUNIT_TEST(set_1);
    CPPUNIT_TEST(set_2);
    CPPUNIT_TEST(set_3);

    CPPUNIT_TEST(get_0);
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST(get_2);
    CPPUNIT_TEST(get_3);

    CPPUNIT_TEST(readConfig_0);

    CPPUNIT_TEST_SUITE_END();


  public:
    void setUp() {
      // create the control hardware
      CControlHardware* hdwr = (new WienerMDGG16::CControlHdwr);

      // create control module and pass ownership of hardware to the 
      // CControlModule

      m_pMod = new CControlModule("test", hdwr);
      m_pMod->configure("-base","0xff000000");
    }
    void tearDown() {
    }

  protected:
    void onAttach_0();
    void initialize_0();
    void initialize_1();

    void set_0();
    void set_1();
    void set_2();
    void set_3();

    void get_0();
    void get_1();
    void get_2();
    void get_3();

    void readConfig_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CMDGG16ControlTests);


class FileJanitor {
  std::string m_path;
  
  public:
  FileJanitor(std::string path) : m_path(path) {}
  FileJanitor(const FileJanitor&) = delete;
  ~FileJanitor() {
    std::remove(m_path.c_str());
  }
};

void generateTestConfigFile (std::string path) {
  std::ofstream f(path.c_str());
  f << "Configuration file for MDGG16Control" << endl;
  f << "Wed Dec 31 16:23:12 EST 2014" << endl;
  f << "or_a " << 255 << endl;
  f << "or_b " << 254 << endl;
  f << "or_c " << 253 << endl;
  f << "or_d " << 252 << endl;
  f << " 0 : Ch0" << endl;
  f << " 1 : Ch1" << endl;
  f << " 2 : Ch2" << endl;
  f << " 3 : Ch3" << endl;
  f << " 4 : Ch4" << endl;
  f << " 5 : Ch5" << endl;
  f << " 6 : Ch6" << endl;
  f << " 7 : Ch7" << endl;
  f << " 8 : Ch8" << endl;
  f << " 9 : Ch9" << endl;
  f << "10 : Ch10" << endl;
  f << "11 : Ch11" << endl;
  f << "12 : Ch12" << endl;
  f << "13 : Ch13" << endl;
  f << "14 : Ch14" << endl;
  f << "15 : Ch15" << endl;
  f.close();
}

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

// if the flags do not exist, then XXUSB::CConfigurableObject will throw.
// This just checks to see that after we have attached our hardware
// to the m_pMod control module, that the required flags have been added and 
// are locatable.
void CMDGG16ControlTests::onAttach_0() {
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-base"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-mode"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_a"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_b"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_c"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-or_d"));
  CPPUNIT_ASSERT_NO_THROW(m_pMod->cget("-configfile"));
}

// Check that we add the proper commands to the stack given a specific set of
// or values in the explicit mode.
void CMDGG16ControlTests::initialize_0() {
  m_pMod->configure("-mode","explicit");
 
 
  m_pMod->configure("-or_a","255");
  m_pMod->configure("-or_b","0");
  m_pMod->configure("-or_c","1");
  m_pMod->configure("-or_d","2");

  CMockVMUSB ctlr;
  m_pMod->Initialize(ctlr);

  vector<string> expected = {
    "executeList::begin",
    "addWrite32 ff00000c 39 858993459", // (i.e. 0x33333333) for ECL outs
    "addWrite32 ff0000d0 39 1717973520", // (i.e. 0x66660000) for NIM outs
    "addWrite32 ff0000b8 39 255",
    "addWrite32 ff0000bc 39 131073",
    "executeList::end"};

  auto record = ctlr.getOperationRecord();

  //print_vectors(expected, record);
  CPPUNIT_ASSERT(expected == record);
  
}


// Same as initialize_0 except that this tests for the proper functioning when
// the -mode is "file"
void CMDGG16ControlTests::initialize_1() {
  // create a test file
  generateTestConfigFile(".testfile.txt");
  FileJanitor janitor(".testfile.txt"); // to cleanup when done

  m_pMod->configure("-mode","file");
  m_pMod->configure("-configfile",".testfile.txt");

  CMockVMUSB ctlr;
  m_pMod->Initialize(ctlr);

  vector<string> expected = {
    "executeList::begin",
    "addWrite32 ff00000c 39 858993459",  // (i.e. 0x33333333)
    "addWrite32 ff0000d0 39 1717973520", // (i.e. 0x66660000) for NIM outs
    "addWrite32 ff0000b8 39 16646399",   // or_a and or_b
    "addWrite32 ff0000bc 39 16515325",   // or_c and or_d
    "executeList::end"};

  auto record = ctlr.getOperationRecord();

  //print_vectors(expected, record);
  CPPUNIT_ASSERT(expected == record);
  
}

// setting the or_ab register does what we expect
void CMDGG16ControlTests::set_0() 
{
  CMockVMUSB ctlr;

  m_pMod->Set(ctlr,"or_ab", "0xa0a0");

  vector<string> expected = {"executeList::begin",
                             "addWrite32 ff0000b8 39 41120",
                             "executeList::end"};

  auto record = ctlr.getOperationRecord();
//  print_vectors(expected, record);
  
  CPPUNIT_ASSERT(expected == record);
}


// setting the or_cd register does what we expect
void CMDGG16ControlTests::set_1() 
{
  CMockVMUSB ctlr;

  m_pMod->Set(ctlr,"or_cd", "0xa0a0");

  vector<string> expected = {"executeList::begin",
                             "addWrite32 ff0000bc 39 41120",
                             "executeList::end"};

  auto record = ctlr.getOperationRecord();
//  print_vectors(expected, record);
  
  CPPUNIT_ASSERT(expected == record);
}


// setting an unknown parameter name throws.
void CMDGG16ControlTests::set_2() 
{
  CMockVMUSB ctlr;


  CPPUNIT_ASSERT_EQUAL( string("ERROR - invalid parameter name \"invalidparam\""),
                                m_pMod->Set(ctlr, "invalidparam", "0"));

}

// see that we don't totally fail when executeList returns error code
// - this is not necessarily a failure mode to break the slow-controls server
//   and may be recoverable.
void CMDGG16ControlTests::set_3() 
{
  CMockVMUSB ctlr;
  ctlr.addReturnDatum(0,-1); // force negative return status

  CPPUNIT_ASSERT_EQUAL( string("ERROR - executeList returned status = -1"),
                                m_pMod->Set(ctlr, "or_ab", "0"));

}

// retrieving content of or_ab does what we expect
void CMDGG16ControlTests::get_0() 
{
  CMockVMUSB ctlr;

  std::string result = m_pMod->Get(ctlr,"or_ab");

  vector<string> expected = {"executeList::begin",
                             "addRead32 ff0000b8 39",
                             "executeList::end"};

  CPPUNIT_ASSERT(expected == ctlr.getOperationRecord());
}


// retrieving content of or_cd register does what we expect
void CMDGG16ControlTests::get_1() 
{
  CMockVMUSB ctlr;

  std::string result = m_pMod->Get(ctlr,"or_cd");

  vector<string> expected = {"executeList::begin",
                             "addRead32 ff0000bc 39",
                             "executeList::end"};

  CPPUNIT_ASSERT(expected == ctlr.getOperationRecord());
}

// tests that we throw when an unknown param name is passed
void CMDGG16ControlTests::get_2() 
{
  CMockVMUSB ctlr;

  CPPUNIT_ASSERT_EQUAL( string("ERROR - invalid parameter name \"invalidparam\""),
                                m_pMod->Get(ctlr, "invalidparam"));
}


// see that we don't totally fail when executeList returns error code
// - this is not necessarily a failure mode to break the slow-controls server
//   and may be recoverable.
void CMDGG16ControlTests::get_3() 
{
  CMockVMUSB ctlr;
  ctlr.addReturnDatum(0,-1); // force negative return status

  CPPUNIT_ASSERT_EQUAL( string("ERROR - executeList returned status = -1"),
                                m_pMod->Get(ctlr, "or_ab"));

}

// basic test for the config file reader.
void CMDGG16ControlTests::readConfig_0() {

  generateTestConfigFile(".testfile.txt");

  // should delete test file when it goes out of scope
  FileJanitor janitor(".testfile.txt");

  using namespace WienerMDGG16;
  CControlHdwrState state = ConfigFileReader().parse(".testfile.txt");

  CPPUNIT_ASSERT_EQUAL(uint32_t(255), state.or_a);
  CPPUNIT_ASSERT_EQUAL(uint32_t(254), state.or_b);
  CPPUNIT_ASSERT_EQUAL(uint32_t(253), state.or_c);
  CPPUNIT_ASSERT_EQUAL(uint32_t(252), state.or_d);

}

