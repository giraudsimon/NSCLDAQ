#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <string>
#include <iostream>
#include <FakeGlobals.h>


using namespace std;

class TCLApplication;
TCLApplication* gpTCLApplication = 0;

int main(int argc, char** argv)
{
  CppUnit::TextUi::TestRunner   
               runner; // Control tests.
  CppUnit::TestFactoryRegistry& 
               registry(CppUnit::TestFactoryRegistry::getRegistry());

  runner.addTest(registry.makeTest());

  bool wasSucessful;
  try {
    wasSucessful = runner.run("",false);
  } 
  catch(string& rFailure) {
    cerr << "Caught a string exception from test suites.: \n";
    cerr << rFailure << endl;
    wasSucessful = false;
  }
  return !wasSucessful;
}
////////// Stubs ////

class CTheApplication {
private:
  static CTheApplication* m_pInstance;
  CTheApplication() {}
  ~CTheApplication() {}
public:
  static   CTheApplication* getInstance();
  void logStateChangeRequest(const char* msg);
  void logStateChangeStatus(const char* msg);
  void logProgress(const char* msg);
  void AcquisitionErrorHandler(Tcl_Event* pEvent, int flags);
};
CTheApplication* CTheApplication::m_pInstance(nullptr);
void CTheApplication::logStateChangeRequest(const char* msg) {}
void CTheApplication::logStateChangeStatus(const char* msg) {}
void CTheApplication::logProgress(const char* msg) {}
void CTheApplication::AcquisitionErrorHandler(Tcl_Event* pEvent, int flags) {}

CTheApplication* CTheApplication::getInstance() {
    if (!m_pInstance) m_pInstance = new CTheApplication;
    return m_pInstance;
  }
