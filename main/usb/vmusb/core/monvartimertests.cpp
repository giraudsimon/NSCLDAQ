// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CMonVar.h"
#undef private


#include <tcl.h>
#include "DataBuffer.h"
#include "CBufferQueue.h"
#include <list>
#include <DataFormat.h>

#include <TCLInterpreter.h>
#include <TCLVariable.h>
#include <sstream>

#include <string.h>

class monvartimertests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(monvartimertests);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(avar);
  CPPUNIT_TEST(twovar);
  CPPUNIT_TEST(split);
  CPPUNIT_TEST(undef);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterp;
  CMonitorVariables* m_pMon;
public:
  void setUp() {
    // Make a fewe buffers and put them in the gFreeBuffers queue.
    
    for (int i =0; i < 10; i++)  {
      DataBuffer* p = createDataBuffer(1024);
      gFreeBuffers.queue(p);
    }
    m_pInterp = new CTCLInterpreter();
    m_pMon    = new CMonitorVariables(*m_pInterp, 100);  // Short timer is fine.
  }
  void tearDown() {
    // Clear the buffer queues:
    
    std::list<DataBuffer*> bufs = gFilledBuffers.getAll();
    bufs.splice(bufs.end(), gFreeBuffers.getAll());
    
    while(!bufs.empty()) {
      delete bufs.front();
      bufs.pop_front();
    }
    
    delete m_pInterp;
    m_pMon->Clear();             // Shouldn't need to but...
    delete m_pMon;
    delete CMonvarDictionary::m_pInstance;
    CMonvarDictionary::m_pInstance = 0;
  }
protected:
  void empty();
  void avar();
  void twovar();
  void split();
  void undef();
private:
  void waitTick();
};


CPPUNIT_TEST_SUITE_REGISTRATION(monvartimertests);


// Utilities

void monvartimertests::waitTick()
{
  
  Tcl_DoOneEvent(TCL_TIMER_EVENTS);
}

//////////////////////////////////////////////////////////////////////
void monvartimertests::empty() {
  // If we don't have any variables to monitor nothing will happen
  // on the tick:
  
  m_pMon->Set();                 // Tell it to go...
  waitTick();
  DataBuffer* pBuf(0);
  bool gotone = gFilledBuffers.getnow(pBuf);
  if (gotone) {
    delete pBuf;
    FAIL(" Got a buffer but nothing to monitor");
  }
}

void monvartimertests::avar()
{
  // Create and set a variable:
  
  CTCLVariable var(m_pInterp, "avar", TCLPLUS::kfFALSE);
  var.Set("SomeValue");
  
  CMonvarDictionary::getInstance()->add("avar");
  
  // All set start the timer for a push:
  
  m_pMon->Set();
  waitTick();                    // Wait for the timer.
  
  std::list<DataBuffer*> bufs = gFilledBuffers.getAll();
  EQ(size_t(1), bufs.size());
  pStringsBuffer p = reinterpret_cast<pStringsBuffer>(
    bufs.front()->s_rawData
  );
  
  EQ(uint32_t(1), p->s_stringCount);
  EQ(uint32_t(MONITORED_VARIABLES), p->s_ringType);
  std::string scriptlet(p->s_strings);
  EQ(std::string("set avar SomeValue"), scriptlet);
  
  delete bufs.front();
}

void monvartimertests::twovar()
{
  CTCLVariable var(m_pInterp, "avar", TCLPLUS::kfFALSE);
  var.Set("SomeValue");
  CMonvarDictionary::getInstance()->add("avar");
  
  CTCLVariable v2(m_pInterp, "bvar", TCLPLUS::kfFALSE);
  v2.Set("anotherValue");
  CMonvarDictionary::getInstance()->add("bvar");
  
  m_pMon->Set();
  waitTick();
  
  std::list<DataBuffer*> bufs = gFilledBuffers.getAll();
  EQ(size_t(1), bufs.size());
  pStringsBuffer p = reinterpret_cast<pStringsBuffer>(
    bufs.front()->s_rawData
  );
  
  EQ(uint32_t(2), p->s_stringCount);
  const char* pStrings = p->s_strings;
  
  std::string s1(pStrings);
  pStrings += s1.size() + 1;
  std::string s2(pStrings);
  
  EQ(std::string("set avar SomeValue"), s1);
  EQ(std::string("set bvar anotherValue"), s2);
  
  delete bufs.front();
}

// Ensure that if necessary the data get split across multiple items.

void monvartimertests::split()
{
  size_t size = 0;
  int i;
  while(size < 1100) {
    std::stringstream strvarname;
    strvarname << "var" << i;
    std::string vname(strvarname.str());
    CTCLVariable v(m_pInterp, vname, TCLPLUS::kfFALSE);
    v.Set("a");
    
    CMonvarDictionary::getInstance()->add(vname);
    
    // Figure out script size and add to size.
 
    size += vname.size();
    size += strlen("set  a") + 1;         // +1 is the null terminator.
 
    i++;   
  }
  
  m_pMon->Set();
  waitTick();
  
  // Should have formatted two items.
  
  std::list<DataBuffer*> bufs =   gFilledBuffers.getAll();
  EQ(size_t(2), bufs.size());
  delete bufs.front();
  delete bufs.back();
}


// Undefined variable should give 'set vname *UNDEFINED*'

void monvartimertests::undef()
{
  CMonvarDictionary::getInstance()->add("junk");
  m_pMon->Set();
  waitTick();
  
    // Should have formatted two items.
  
  std::list<DataBuffer*> bufs =   gFilledBuffers.getAll();
  EQ(size_t(1), bufs.size());
  pStringsBuffer pS = reinterpret_cast<pStringsBuffer>(
    bufs.front()->s_rawData
  );
  
  std::string scriptlet(pS->s_strings);
  EQ(std::string("set junk *UNDEFINED*"), scriptlet);
}
