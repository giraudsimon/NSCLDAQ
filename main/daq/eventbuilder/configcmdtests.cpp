// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CFragmentHandler.h"
#undef private

#include <TCLInterpreter.h>
#include "CConfigure.h"
#include <stdlib.h>
#include <fragment.h>



class ConfigCmdTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ConfigCmdTest);
  CPPUNIT_TEST(setxon);
  CPPUNIT_TEST(setxoff);
//  CPPUNIT_TEST(xoffObserved);
//  CPPUNIT_TEST(xonObserved);
  CPPUNIT_TEST_SUITE_END();


private:
    CTCLInterpreter* m_pInterp;
    CFragmentHandler* m_pHandler;
public:
  void setUp() {
    m_pInterp = new CTCLInterpreter();
    m_pHandler = new CFragmentHandler();
  }
  void tearDown() {
    delete m_pHandler;
    delete m_pInterp;
  }
protected:
  void setxon();
  void setxoff();
  void xoffObserved();
  void xonObserved();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigCmdTest);

void ConfigCmdTest::setxon() {
    CConfigure cmd(*m_pInterp, "config");
    
    m_pInterp->Eval("config set XonThreshold 1234");
    EQ(static_cast<size_t>(1234), m_pHandler->m_nXonLimit);
}
void ConfigCmdTest::setxoff() {
    CConfigure cmd(*m_pInterp, "config");
    
    m_pInterp->Eval("config set XoffThreshold 1234");
    EQ(static_cast<size_t>(1234), m_pHandler->m_nXoffLimit);
}

class XonOffObserver : public CFragmentHandler::FlowControlObserver {
public:
    bool m_fXoffed;
    bool m_fXoned;
    XonOffObserver() :m_fXoffed(false), m_fXoned(false) {}
    void Xoff() {m_fXoffed = true;}
    void Xon()  {m_fXoned = true;}
    void Xoff(std::string qid) {m_fXoffed = true;}
    void Xon(std::string qid) {m_fXoned = true;}
};

static void initFragment(EVB::pFlatFragment frag)
{
    frag->s_header.s_timestamp = 100;              // nonzero is the key.
    frag->s_header.s_sourceId  = 1;
    frag->s_header.s_size      = 10;
    frag->s_header.s_barrier   = 0;
}
void ConfigCmdTest::xoffObserved()
{
    XonOffObserver obs;
    m_pHandler->addFlowControlObserver(&obs);
    m_pHandler->setXoffThreshold(1);           // Anything makes the threshold.
    m_pHandler->setBuildWindow(0);
    
    EVB::FlatFragment *frag =
        reinterpret_cast<EVB::FlatFragment*>(
            malloc(sizeof(EVB::FlatFragment) + 10));

    initFragment(frag);
    
    // Submitting this fragment should invoke the Xoff observer
    
    m_pHandler->addFragments(sizeof(EVB::FlatFragment) + 10, frag);
    free(frag);
    
    ASSERT(obs.m_fXoffed);
}

void ConfigCmdTest::xonObserved()
{
    XonOffObserver obs;
    m_pHandler->addFlowControlObserver(&obs);
    m_pHandler->setXoffThreshold(5);           // Anything makes the threshold.
    m_pHandler->setXonThreshold(1);
    
    EVB::FlatFragment *frag =
        reinterpret_cast<EVB::FlatFragment*>(
            malloc(sizeof(EVB::FlatFragment) + 10));

    initFragment(frag);
    
    // Submitting this fragment should invoke the Xoff observer
    
    m_pHandler->addFragments(sizeof(EVB::FlatFragment) + 10, frag);
    
    // Flushing should xon
    
    m_pHandler->flushQueues();
    
    ASSERT(obs.m_fXoned);
    
}