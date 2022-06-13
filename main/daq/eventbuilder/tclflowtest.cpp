// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CFragmentHandler.h"
#undef private

#include "CXonXOffCallbackCommand.h"
#include "TCLInterpreter.h"
#include "TCLVariable.h"
#include "fragment.h"
#include "TCLException.h"




class TclFlowTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TclFlowTest);
  CPPUNIT_TEST(establish);
//  CPPUNIT_TEST(foffcalled);
//  CPPUNIT_TEST(foncalled);
  CPPUNIT_TEST(remove);
  CPPUNIT_TEST(removenosuch);
  CPPUNIT_TEST_SUITE_END();


private:
    CTCLInterpreter* m_pInterp;
    CXonXoffCallbackCommand* m_pCommand;
    
public:
  void setUp() {
    m_pInterp = new CTCLInterpreter();
    CFragmentHandler::m_pInstance = new CFragmentHandler;
    m_pCommand = new CXonXoffCallbackCommand(*m_pInterp, "flowcontrol");
  }
  void tearDown() {
    delete CFragmentHandler::m_pInstance;
    CFragmentHandler::m_pInstance = 0;
    delete m_pInterp;
    delete m_pCommand;
  }
protected:
  void establish();
  void foffcalled();
  void foncalled();
  void remove();
  void removenosuch();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TclFlowTest);

/*
 * Adding a script will add an entry to the fragment handler's observer list.
 */
void TclFlowTest::establish() {
    m_pInterp->Eval("flowcontrol add test test");
    EQ(size_t(1), CFragmentHandler::getInstance()->m_flowControlObservers.size());
}

static void initFragment(EVB::pFlatFragment pFrag)
{
    pFrag->s_header.s_timestamp = 100;
    pFrag->s_header.s_sourceId  =   1;
    pFrag->s_header.s_size      =  10;
    pFrag->s_header.s_barrier   =   0;
}

/*
 *  Adding a flow off script and forcing flow off will invoke the script.
 */
void
TclFlowTest::foffcalled()
{
    m_pInterp->Eval("set ::flow on");
    m_pInterp->Eval("flowcontrol add test {set ::flow off}");
    CFragmentHandler* p = CFragmentHandler::getInstance();
    p->setXoffThreshold(5);
    EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(malloc(sizeof(EVB::FlatFragment) + 10));
    initFragment(pFrag);
    
    p->addFragments(sizeof(EVB::FlatFragment) + 10, pFrag);          // Shouild flow off.
    CTCLVariable flow(m_pInterp, "::flow", false);
    EQ(std::string("off"), std::string(flow.Get()));
    
    free(pFrag);
    
}



void TclFlowTest::foncalled()
{
    m_pInterp->Eval("set ::flow on");
    m_pInterp->Eval("flowcontrol add {set ::flow on} {set ::flow off}");
    CFragmentHandler* p = CFragmentHandler::getInstance();
    p->setXoffThreshold(5);
    EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(malloc(sizeof(EVB::FlatFragment) + 10));
    initFragment(pFrag);
    
    p->addFragments(sizeof(EVB::FlatFragment) + 10, pFrag);          // Shouild flow off.
    CTCLVariable flow(m_pInterp, "::flow", false);
    EQ(std::string("off"), std::string(flow.Get()));
    p->flushQueues();                                                // should flow back on.
    
    
    EQ(std::string("on"), std::string(flow.Get()));
    
    free(pFrag);    
}
/*
 *  Should be able to remove the item.
 */
void TclFlowTest::remove()
{
    m_pInterp->Eval("flowcontrol add test test");
    EQ(size_t(1), CFragmentHandler::getInstance()->m_flowControlObservers.size());
    
    m_pInterp->Eval("flowcontrol remove test test");
    EQ(size_t(0), CFragmentHandler::getInstance()->m_flowControlObservers.size());
}
/*
 * It's an error to remove an item that does not exist.
 */
void TclFlowTest::removenosuch()
{
    bool thrown = false;
    bool threwright = true;
    try {
        m_pInterp->Eval("flowcontrol remove test test");
    }
    catch (CTCLException& e) {
        thrown = true;
        threwright = true;
    }
    catch (...) {
        thrown = true;
    }
    ASSERT(thrown);
    ASSERT(threwright);
}