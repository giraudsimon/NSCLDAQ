

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iterator>
#include <algorithm>
#include <CMockVMUSB.h>
#include <CLoggingReadoutList.h>
#include <CControlModule.h>

#define private public
#define protected public
#include <CMxDCRCBus.h>
#undef protected
#undef private

using namespace std;

class CMxDCRCBusTests : public CppUnit::TestFixture {
  private:
    CMxDCRCBus* m_pHdwr;
    CControlModule* m_pModule;
    unique_ptr<CMockVMUSB> m_pCtlr;

  public:
    CPPUNIT_TEST_SUITE(CMxDCRCBusTests);
    CPPUNIT_TEST(clone_0);
    CPPUNIT_TEST(set_0);
    CPPUNIT_TEST(get_0);
    CPPUNIT_TEST(initialize_0);
    CPPUNIT_TEST(activate_0);
    CPPUNIT_TEST(pollForResponse_0);
    CPPUNIT_TEST(pollForResponse_1);
    CPPUNIT_TEST(readResult_0);
    CPPUNIT_TEST(parseAddress_0);
    CPPUNIT_TEST(addParameterWrite_0);
    CPPUNIT_TEST(addParameterRead_0);
    CPPUNIT_TEST(responseIndicatesError_0);
    CPPUNIT_TEST(responseIndicatesError_1);
    CPPUNIT_TEST(convertResponseToErrorString_0);
    CPPUNIT_TEST(convertResponseToErrorString_1);
    CPPUNIT_TEST(convertResponseToErrorString_2);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
     m_pHdwr = new CMxDCRCBus;
    m_pModule =    new CControlModule("test", m_pHdwr);

    m_pModule->configure("-base","0xff000000");

    m_pCtlr = unique_ptr<CMockVMUSB>(new CMockVMUSB);
  }
  void tearDown() {
  }
protected:
  // public interface tests
  void clone_0();
  void set_0();
  void get_0();
  void initialize_0();

  // tests for helper functions
  void activate_0();
  void pollForResponse_0();
  void pollForResponse_1();
  void readResult_0();
  void parseAddress_0();
  void addParameterWrite_0();
  void addParameterRead_0();
  void responseIndicatesError_0();
  void responseIndicatesError_1();
  void convertResponseToErrorString_0();
  void convertResponseToErrorString_1();
  void convertResponseToErrorString_2();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CMxDCRCBusTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


void CMxDCRCBusTests::clone_0() 
{
  size_t newPollTimeout = 12;
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());

  CPPUNIT_ASSERT( newPollTimeout != hdwr->getPollTimeout() );
  CPPUNIT_ASSERT( nullptr != hdwr->getConfiguration() );

  CMxDCRCBus other;
  CPPUNIT_ASSERT( nullptr == other.getConfiguration() );
  other.setPollTimeout(newPollTimeout);

  *hdwr = other;

  CPPUNIT_ASSERT_EQUAL(newPollTimeout, hdwr->getPollTimeout());
  CPPUNIT_ASSERT(nullptr == hdwr->getConfiguration());
}


void CMxDCRCBusTests::activate_0() {
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  hdwr->activate(*m_pCtlr);

  vector<string> record = m_pCtlr->getOperationRecord();
  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff00606e 09 3";
  expected[2] = "executeList::end";

  CPPUNIT_ASSERT(expected == record);
}


void CMxDCRCBusTests::set_0() 
{
  // fill the return data
  m_pCtlr->addReturnDatum(1);
  m_pCtlr->addReturnDatum(48);
  m_pCtlr->addReturnDatum(48);

  // provide dev number, address
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  string result = hdwr->Set(*m_pCtlr, "d7a24", "48");

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(12);
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff006082 09 7";
  expected[2] = "addWrite16 ff006084 09 16";
  expected[3] = "addWrite16 ff006086 09 24";
  expected[4] = "addWrite16 ff006088 09 48";
  expected[5] = "executeList::end";
  // then read for data as single shot commands..
  expected[6] = "executeList::begin";
  expected[7] = "addRead16 ff00608a 09";
  expected[8] = "executeList::end";

  expected[9] = "executeList::begin";
  expected[10] = "addRead16 ff006088 09";
  expected[11] = "executeList::end";

  CPPUNIT_ASSERT( expected == record );
}


void CMxDCRCBusTests::get_0() 
{
  // setup what we expect the VM-USB to return to us
  m_pCtlr->addReturnDatum(0, 0); // set up read
  m_pCtlr->addReturnDatum(0, 0); // poll for response

  // execute the Get command
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  std::string retStr = hdwr->Get(*m_pCtlr, "d7a24");

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(12);
  // setup and start transaction
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff006082 09 7";  // dev number
  expected[2] = "addWrite16 ff006084 09 18"; // read operation
  expected[3] = "addWrite16 ff006086 09 24"; // param ddress
  expected[4] = "addWrite16 ff006088 09 0"; // initiate transaction
  expected[5] = "executeList::end";

  // read response
  expected[6] = "executeList::begin";
  expected[7] = "addRead16 ff00608a 09";
  expected[8] = "executeList::end";

  expected[9] = "executeList::begin";
  expected[10] = "addRead16 ff006088 09";
  expected[11] = "executeList::end";

  
  CPPUNIT_ASSERT(expected == record);
}


void CMxDCRCBusTests::pollForResponse_0() 
{
  // provide dev number, address
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  hdwr->pollForResponse(*m_pCtlr);

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "addRead16 ff00608a 09";
  expected[2] = "executeList::end";

//  print_vectors(expected,record);

  CPPUNIT_ASSERT(expected==record);
}

// Test that we can timeout properly after 1000 tries
void CMxDCRCBusTests::pollForResponse_1() 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());

  // setup the controller to not return 0 for more than the allowed poll
  // attempts
  size_t maxPolls = hdwr->getPollTimeout();
  for (size_t iter=0; iter<maxPolls+1; ++iter) m_pCtlr->addReturnDatum(1);

  CPPUNIT_ASSERT_THROW(hdwr->pollForResponse(*m_pCtlr),
                       std::string);
}


void CMxDCRCBusTests::readResult_0() 
{
  m_pCtlr->addReturnDatum(0);

  // execute the Get command
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  hdwr->readResult(*m_pCtlr);

  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "addRead16 ff006088 09";
  expected[2] = "executeList::end";

  CPPUNIT_ASSERT( expected == m_pCtlr->getOperationRecord());
}

void CMxDCRCBusTests::parseAddress_0()
{
  // provide dev number, address
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  auto address = hdwr->parseAddress("d123a4567");

  CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(123), address.first);
  CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(4567), address.second);
}


void CMxDCRCBusTests::addParameterWrite_0() 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());

  // create a logging list
  CLoggingReadoutList list;

  // define the dev number (24), and address (56), some data to write
  std::pair<uint16_t,uint16_t> addresses(24,56);
  uint16_t dataToWrite = 124;

  // perform the operation
  hdwr->addParameterWrite(list, addresses, dataToWrite);

  vector<string> expected(4);
  expected[0] = "addWrite16 ff006082 09 24"; // device num
  expected[1] = "addWrite16 ff006084 09 16"; // opcode=write
  expected[2] = "addWrite16 ff006086 09 56"; // param address
  expected[3] = "addWrite16 ff006088 09 124"; // value & initiate transaction

  CPPUNIT_ASSERT(expected == list.getLog());
  
}

void CMxDCRCBusTests::addParameterRead_0() 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());

  // create a logging list
  CLoggingReadoutList list;

  // define the dev number (24), and address (56), some data to write
  std::pair<uint16_t,uint16_t> addresses(24,56);
  uint16_t dataToWrite = 124;

  // perform the operation
  hdwr->addParameterRead(list, addresses);

  vector<string> expected(4);
  expected[0] = "addWrite16 ff006082 09 24"; // device number
  expected[1] = "addWrite16 ff006084 09 18"; // opcode=read
  expected[2] = "addWrite16 ff006086 09 56"; // param address
  expected[3] = "addWrite16 ff006088 09 0";  // initiate transaction

  CPPUNIT_ASSERT(expected == list.getLog());
  
}

void CMxDCRCBusTests::responseIndicatesError_0()
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  CPPUNIT_ASSERT_EQUAL(false, hdwr->responseIndicatesError(0));
}

void CMxDCRCBusTests::responseIndicatesError_1()
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  CPPUNIT_ASSERT_EQUAL(true, hdwr->responseIndicatesError(2));
}

void CMxDCRCBusTests::convertResponseToErrorString_0 () 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  string expected = "ERROR - Address collision during last RC-bus operation";
  expected += " : code=2";
  CPPUNIT_ASSERT_EQUAL(expected, hdwr->convertResponseToErrorString(2));
}

void CMxDCRCBusTests::convertResponseToErrorString_1 () 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  std::string expected = "ERROR - No response during last RC-bus operation";
  expected += " : code=4";
  CPPUNIT_ASSERT_EQUAL(expected, hdwr->convertResponseToErrorString(4));
}

void CMxDCRCBusTests::convertResponseToErrorString_2 () 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  std::string expected = "ERROR - Unknown error code returned from last ";
  expected += "RC-bus operation : code=8";
  // i don't really know if multiple error bits can be specified, but 
  // I will test a condition that should be independent. For safety, I
  // will test what happens when an unknown bit is set.
  CPPUNIT_ASSERT_EQUAL(expected, hdwr->convertResponseToErrorString(8));
}


void CMxDCRCBusTests::initialize_0 ()
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  hdwr->Initialize(*m_pCtlr);

  vector<string> record = m_pCtlr->getOperationRecord();
  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff00606e 09 3";
  expected[2] = "executeList::end";

  CPPUNIT_ASSERT(expected == record);

}

