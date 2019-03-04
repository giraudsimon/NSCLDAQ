// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string>
#include "CTriggerLoop.h"
#include "CEventTrigger.h"
#include "CExperiment.h"
#include <unistd.h>

class CEventSegment;
class CScaler;
class CBusy;

using namespace std;

// test globals:

unsigned scalerTriggers;
unsigned eventTriggers;

// Test class

class TriggerLoopTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TriggerLoopTests);
  CPPUNIT_TEST(theTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    scalerTriggers = eventTriggers = 0;
  }
  void tearDown() {
  }
protected:
  void theTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TriggerLoopTests);



// Test trigger that is always set:

class TestTrigger : public CEventTrigger
{
  virtual bool operator()() { return true; }
};



// Below is a fixture for CExperiment:




CExperiment::CExperiment(string name, size_t bufsize) :
  m_pEventTrigger(new TestTrigger),
  m_pScalerTrigger(new TestTrigger)
 {}

CExperiment::~CExperiment()
{
  delete m_pEventTrigger;
  delete m_pScalerTrigger;
}

void CExperiment::setBufferSize(size_t newSize) {}
size_t CExperiment::getBufferSize() const { return 0;}
void CExperiment::Start(bool resume) {}
void CExperiment::Stop(bool pause) {}

void CExperiment::AddEventSegment(CEventSegment* pSegment) {}
void CExperiment::RemoveEventSegment(CEventSegment* pSegment) {} 

void CExperiment::AddScalerModule(CScaler* pModule) {}
void CExperiment::RemoveScalerModule(CScaler* pModule){}

void CExperiment::EstablishTrigger(CEventTrigger* pEventTrigger) {}
void CExperiment::setScalerTrigger(CEventTrigger* pScalerTrigger) {}
void CExperiment::EstablishBusy(CBusy*  pBusyModule) {}

void CExperiment::ReadEvent() {
  eventTriggers++;
}
void CExperiment::TriggerScalerReadout() {
  scalerTriggers++;
}
void CExperiment::DocumentPackets() {}
void CExperiment::ScheduleRunVariableDump() {}

CEventTrigger* CExperiment::getEventTrigger() { 
  return m_pEventTrigger;
}
CEventTrigger* CExperiment::getScalerTrigger() {
  return m_pScalerTrigger;
}

//////////////////////////////////////////////////////////////////////////////

// Starting should ensure that at least one event trigger and at least one scaler
// trigger will fire.

void TriggerLoopTests::theTest() {
  CExperiment theExperiment("Test");
  CTriggerLoop theTrigger(theExperiment);
  
  theTrigger.start();
  sleep(1);
  theTrigger.stop(pause);

  ASSERT(scalerTriggers != 0);
  ASSERT(eventTriggers  != 0);
}
