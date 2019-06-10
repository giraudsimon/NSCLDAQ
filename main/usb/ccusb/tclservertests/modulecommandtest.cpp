

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>

#define private public
#define protected public
#include <CModuleCommand.h>
#undef protected
#undef private

using namespace std;

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CCtlConfiguration.h>


class CModuleCommandTests : public CppUnit::TestFixture {
  private:
    unique_ptr<CCtlConfiguration> m_server;
    unique_ptr<CTCLInterpreter> m_interp;
    unique_ptr<CModuleCommand> m_cmd;

  public:
    CPPUNIT_TEST_SUITE(CModuleCommandTests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST(create_1);
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(CModuleCommandTests);

void CModuleCommandTests::create_0() {
  CTCLObject arg;
  vector<CTCLObject> arglist(4); 
  arglist[0] = (arg = "Module");
  arglist[1] = (arg = "create");
  arglist[2] = (arg = "ccusb");
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
  arglist[2] = (arg = "ccusb");
  arglist[3] = (arg = "name");

  // create the object the first time
  int status = m_cmd->operator()(*m_interp, arglist);
  CPPUNIT_ASSERT_EQUAL(TCL_OK, status);

  // creating a second object with the same name is an error
  status = m_cmd->operator()(*m_interp, arglist);
  CPPUNIT_ASSERT_EQUAL(TCL_ERROR, status);
}

