
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CReadoutModule.h>
#include <CMockVMUSB.h>
#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>

#define private public
#define protected public
#include <CMQDC32RdoHdwr.h>
#undef protected
#undef private


// Fool the linker
//namespace Globals {
//  ::CConfiguration* pConfig;
//}



using namespace std;

class cmqdc32test : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(cmqdc32test);
  CPPUNIT_TEST( initialize_0 );
  CPPUNIT_TEST_SUITE_END();


private:
  CMQDC32RdoHdwr*  m_pModule;
  CReadoutModule* m_pConfig;

public:
  void setUp() {
    m_pConfig = new CReadoutModule("test", CMQDC32RdoHdwr());
    m_pModule = static_cast<CMQDC32RdoHdwr*>(m_pConfig->getHardwarePointer());
  }
  void tearDown() {
    delete m_pConfig;
  }
protected:
  void initialize_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cmqdc32test);

 
// this is a behavioral test. It merely record the default functionality that
// has been tested to work.
void cmqdc32test::initialize_0()
{
  std::string baseString = "0x80000000";
  m_pConfig->configure("-base", baseString);

  CMockVMUSB ctlr;
  ctlr.addReturnDatum(1);
  ctlr.addReturnDatum(1);
  ctlr.addReturnDatum(1);
  m_pModule->Initialize(ctlr);

  std::vector<std::string> expected = {"executeList::begin",
    "addWrite16 8000603a 09 0",
    "executeList::end",
    "executeList::begin",
    "addWrite16 80006010 09 0",
    "addDelay 1",
    "addWrite16 80006004 09 0",
    "addDelay 1",
    "addWrite16 80006012 09 0",
    "addDelay 1",
    "addWrite16 80006010 09 0",
    "addDelay 1",
    "addWrite16 8000601c 09 1",
    "addDelay 1",
    "addWrite16 80006036 09 0",
    "addDelay 1",
    "addWrite16 80006018 09 1",
    "addDelay 1",
    "addWrite16 8000601a 09 1",
    "addDelay 1",
    "addWrite16 8000604c 09 0",
    "addDelay 1",
    "addWrite16 80004000 09 0",
    "addDelay 1",
    "addWrite16 80004002 09 0",
    "addDelay 1",
    "addWrite16 80004004 09 0",
    "addDelay 1",
    "addWrite16 80004006 09 0",
    "addDelay 1",
    "addWrite16 80004008 09 0",
    "addDelay 1",
    "addWrite16 8000400a 09 0",
    "addDelay 1",
    "addWrite16 8000400c 09 0",
    "addDelay 1",
    "addWrite16 8000400e 09 0",
    "addDelay 1",
    "addWrite16 80004010 09 0",
    "addDelay 1",
    "addWrite16 80004012 09 0",
    "addDelay 1",
    "addWrite16 80004014 09 0",
    "addDelay 1",
    "addWrite16 80004016 09 0",
    "addDelay 1",
    "addWrite16 80004018 09 0",
    "addDelay 1",
    "addWrite16 8000401a 09 0",
    "addDelay 1",
    "addWrite16 8000401c 09 0",
    "addDelay 1",
    "addWrite16 8000401e 09 0",
    "addDelay 1",
    "addWrite16 80004020 09 0",
    "addDelay 1",
    "addWrite16 80004022 09 0",
    "addDelay 1",
    "addWrite16 80004024 09 0",
    "addDelay 1",
    "addWrite16 80004026 09 0",
    "addDelay 1",
    "addWrite16 80004028 09 0",
    "addDelay 1",
    "addWrite16 8000402a 09 0",
    "addDelay 1",
    "addWrite16 8000402c 09 0",
    "addDelay 1",
    "addWrite16 8000402e 09 0",
    "addDelay 1",
    "addWrite16 80004030 09 0",
    "addDelay 1",
    "addWrite16 80004032 09 0",
    "addDelay 1",
    "addWrite16 80004034 09 0",
    "addDelay 1",
    "addWrite16 80004036 09 0",
    "addDelay 1",
    "addWrite16 80004038 09 0",
    "addDelay 1",
    "addWrite16 8000403a 09 0",
    "addDelay 1",
    "addWrite16 8000403c 09 0",
    "addDelay 1",
    "addWrite16 8000403e 09 0",
    "addDelay 1",
    "addWrite16 80006054 09 0",
    "addDelay 1",
    "addWrite16 80006056 09 0",
    "addDelay 1",
    "addWrite16 80006050 09 255",
    "addDelay 1",
    "addWrite16 80006052 09 255",
    "addDelay 1",
    "addWrite16 80006044 09 128",
    "addDelay 1",
    "addWrite16 80006046 09 128",
    "addDelay 1",
    "addWrite16 80006040 09 0",
    "addDelay 1",
    "addWrite16 80006064 09 0",
    "addDelay 1",
    "addWrite16 80006066 09 0",
    "addDelay 1",
    "addWrite16 8000606a 09 0",
    "addDelay 1",
    "addWrite16 8000606c 09 1",
    "addDelay 1",
    "addWrite16 8000606e 09 0",
    "addDelay 1",
    "addWrite16 80006060 09 0",
    "addDelay 1",
    "addWrite16 80006062 09 31",
    "addDelay 1",
    "addWrite16 80006096 09 0",
    "addDelay 1",
    "addWrite16 80006098 09 1",
    "addDelay 1",
    "addWrite16 80006038 09 0",
    "addDelay 1",
    "addWrite16 80006090 09 3",
    "addDelay 1",
    "addWrite16 800060b2 09 0",
    "addDelay 1",
    "addWrite16 800060b6 09 0",
    "addDelay 1",
    "addWrite16 800060b0 09 32",
    "addDelay 1",
    "addWrite16 800060b4 09 16",
    "addDelay 1",
    "addWrite16 80006070 09 0",
    "addDelay 1",
    "addWrite16 8000603c 09 1",
    "addDelay 1",
    "executeList::end",
    "executeList::begin",
    "addWrite16 8000603a 09 1",
    "addWrite16 80006034 09 0",
    "executeList::end"};


  auto ops = ctlr.getOperationRecord();

  CPPUNIT_ASSERT(expected == ops);

}
