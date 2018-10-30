// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#define private public
#include "CMonVar.h"
#undef private

#include "Asserts.h"
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <TCLObject.h>
#include <vector>

class mvarcmdtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(mvarcmdtests);
  CPPUNIT_TEST(add1);
  CPPUNIT_TEST(addinsuf);
  CPPUNIT_TEST(cmdinsuf);
  CPPUNIT_TEST(adddup);
  
  CPPUNIT_TEST(remove);
  CPPUNIT_TEST(removenox);
  CPPUNIT_TEST(removeinsuf);
  
  CPPUNIT_TEST(listnopatt);
  CPPUNIT_TEST(listwpatt);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterp;
  CMonvarCommand*  m_pCmd;
public:
  void setUp() {
    m_pInterp = new CTCLInterpreter;
    m_pCmd    = new CMonvarCommand(*m_pInterp);
  }
  void tearDown() {
    delete m_pCmd;
    delete m_pInterp;
    
    delete CMonvarDictionary::m_pInstance;
    CMonvarDictionary::m_pInstance = 0;
  }
protected:
  void add1();
  void addinsuf();
  void cmdinsuf();
  
  void remove();
  void removeinsuf();
  
  void adddup();
  void removenox();
  
  void listnopatt();
  void listwpatt();
private:
  void stockforlist();
};

CPPUNIT_TEST_SUITE_REGISTRATION(mvarcmdtests);


///////////////////////////////////////////////////////////////////////////////

static std::vector<std::string> listvars = {
  "element1", "element2", "element3", "element31", "element32"
};

  // element3* - update if listvars changes.

static std::vector<std::string> list3 = {
  "element3", "element31", "element32"
};

void
mvarcmdtests::stockforlist()
{
  for (int i = 0; i < listvars.size(); i++) {
    std::string cmd = "monvar add ";
    cmd += listvars[i];
    m_pInterp->GlobalEval(cmd);
  }
}

//////////////////////////////////////////////////////////////////////////////

void mvarcmdtests::add1() {
  
  m_pInterp->GlobalEval("monvar add atest");
  
  const std::list<std::string>& l(CMonvarDictionary::getInstance()->get());
  EQ(size_t(1), l.size());
  EQ(std::string("atest"), l.front());
}

// Need a variable name:

void mvarcmdtests::addinsuf()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterp->GlobalEval("monvar add"),          // missing varname.
    CTCLException
  );
}
// Need a subcommand:

void mvarcmdtests::cmdinsuf()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterp->GlobalEval("monvar"),
    CTCLException
  );
}

void mvarcmdtests::remove()
{
  m_pInterp->GlobalEval("monvar add b");
  m_pInterp->GlobalEval("monvar add a");
  m_pInterp->GlobalEval("monvar add c");   // a b c is order.

  m_pInterp->GlobalEval("monvar remove b");  // a c.
  
  const std::list<std::string>& l(CMonvarDictionary::getInstance()->get());
  EQ(size_t(2), l.size());
  EQ(std::string("a"), l.front());
  EQ(std::string("c"), l.back());
}

void mvarcmdtests::removeinsuf()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterp->GlobalEval("monvar remove"),
    CTCLException
  );
}

// Adding a dup is not legal:

void mvarcmdtests::adddup()
{
  m_pInterp->GlobalEval("monvar add a");
  CPPUNIT_ASSERT_THROW(
    m_pInterp->GlobalEval("monvar add a"),
    CTCLException
  );
}

// Removing an item that does not exist is not legal:

void mvarcmdtests::removenox()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterp->GlobalEval("monvar remove a"),
    CTCLException
  );
}

//  list with no pattern is list *

void mvarcmdtests::listnopatt()
{
  stockforlist();
  
  std::string result = m_pInterp->GlobalEval("monvar list");

  // Simplest way to parse a list:
  
  CTCLObject resultList;
  resultList.Bind(*m_pInterp);
  resultList = result;
  EQ(int(listvars.size()), resultList.llength());
  for (int i =0; i < listvars.size(); i++) {
    CTCLObject o  = resultList.lindex(i);
    o.Bind(*m_pInterp);
    std::string aname = std::string(o);
    
    EQ(aname, listvars[i]);
  }
  
}
void mvarcmdtests::listwpatt()
{
  stockforlist();
  
  std::string result = m_pInterp->GlobalEval("monvar list element3*");
  // Simplest way to parse a list:
  
  CTCLObject resultList;
  resultList.Bind(*m_pInterp);
  resultList = result;

  EQ(int(list3.size()), resultList.llength());
  for (int i =0; i < list3.size(); i++) {
    CTCLObject o  = resultList.lindex(i);
    o.Bind(*m_pInterp);
    std::string aname = std::string(o);
    
    EQ(aname, list3[i]);
  }

}