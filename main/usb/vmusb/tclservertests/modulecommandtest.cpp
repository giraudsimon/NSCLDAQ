

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>

// To avoid instantiating a TclServer instance, we are going to use the
// preprocessor to substitute a CFakeTclServer.
#define private public
#define protected public
#include <CModuleCommand.h>
#undef protected
#undef private

using namespace std;

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CCtlConfiguration.h>

// TclServer is supposed to be a final class, but we just need
// something fake to pass to the constructor. We had to bend its arm
// a bit too by adding a virtual keyword in front of setResult.


class CModuleCommandTests : public CppUnit::TestFixture {
  private:
    unique_ptr<CCtlConfiguration> m_server;
    unique_ptr<CTCLInterpreter> m_interp;
    unique_ptr<CModuleCommand> m_cmd;

  public:
    CPPUNIT_TEST_SUITE(CModuleCommandTests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(createMxDCRCBus_0);
    CPPUNIT_TEST(createChicoTrigger_0);
    CPPUNIT_TEST(createMDGG16Control_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {

    m_server.reset(new CCtlConfiguration); 
    m_interp.reset(new CTCLInterpreter);
    m_cmd.reset(new CModuleCommand(*m_interp, *m_server));
  }
  void tearDown() {
  }
protected:
  void create_0();
  void create_1();
  void createMxDCRCBus_0();
  void createChicoTrigger_0();
  void createMDGG16Control_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CModuleCommandTests);

void CModuleCommandTests::create_0() {
  CTCLObject arg;
  vector<CTCLObject> arglist(4); 
  arglist[0] = (arg = "Module");
  arglist[1] = (arg = "create");
  arglist[2] = (arg = "vmusb");
  arglist[3] = (arg = "name");

  // process the command 
  m_cmd->operator()(*m_interp, arglist);

  // check that the module was created
  CPPUNIT_ASSERT( nullptr != m_server->findModule("name"));
}



void CModuleCommandTests::create_1() {
  CTCLObject arg;
  vector<CTCLObject> arglist(4); 
  arglist[0] = (arg = "Module");
  arglist[1] = (arg = "create");
  arglist[2] = (arg = "vmusb");
  arglist[3] = (arg = "name");

  // create the object the first time
  int status = m_cmd->operator()(*m_interp, arglist);
  CPPUNIT_ASSERT_EQUAL(TCL_OK, status);

  // creating a second object with the same name is an error
  status = m_cmd->operator()(*m_interp, arglist);
  CPPUNIT_ASSERT_EQUAL(TCL_ERROR, status);
}


void CModuleCommandTests::createMxDCRCBus_0() {
  CTCLObject arg;
  vector<CTCLObject> arglist(4); 
  arglist[0] = (arg = "Module");
  arglist[1] = (arg = "create");
  arglist[2] = (arg = "mxdcrcbus");
  arglist[3] = (arg = "name");

  // create the object the first time
  int status = m_cmd->operator()(*m_interp, arglist);
  CPPUNIT_ASSERT_EQUAL(TCL_OK, status);

  CPPUNIT_ASSERT( nullptr != m_server->findModule("name"));
}

void CModuleCommandTests::createChicoTrigger_0() {
  CTCLObject arg;
  vector<CTCLObject> arglist(4); 
  arglist[0] = (arg = "Module");
  arglist[1] = (arg = "create");
  arglist[2] = (arg = "chicotrigger");
  arglist[3] = (arg = "name");

  // create the object the first time
  int status = m_cmd->operator()(*m_interp, arglist);
  CPPUNIT_ASSERT_EQUAL(TCL_OK, status);

  CPPUNIT_ASSERT( nullptr != m_server->findModule("name"));
}

void CModuleCommandTests::createMDGG16Control_0() {
  CTCLObject arg;
  vector<CTCLObject> arglist(4); 
  arglist[0] = (arg = "Module");
  arglist[1] = (arg = "create");
  arglist[2] = (arg = "mdgg16");
  arglist[3] = (arg = "name");

  // create the object the first time
  int status = m_cmd->operator()(*m_interp, arglist);
  CPPUNIT_ASSERT_EQUAL(TCL_OK, status);

  CPPUNIT_ASSERT( nullptr != m_server->findModule("name"));
}










