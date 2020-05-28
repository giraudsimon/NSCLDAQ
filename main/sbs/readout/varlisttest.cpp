// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <TCLInterpreter.h>
#include "CVarList.h"
#include <vector>
#include <string>

using namespace std;

size_t zero(0);

class VarLTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VarLTest);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(add);
  CPPUNIT_TEST(remove);
  CPPUNIT_TEST(find);
  CPPUNIT_TEST(iterate);
  CPPUNIT_TEST(formatnotset);
  CPPUNIT_TEST(formatset);
  CPPUNIT_TEST(specialchars);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterp;
  CVarList*        m_pList;
public:
  void setUp() {
    m_pInterp = new CTCLInterpreter();
    m_pList   = new CVarList(*m_pInterp);
  }
  void tearDown() {
    delete m_pList;
    delete m_pInterp;
  }
protected:
  void construct();
  void add();
  void remove();
  void find();
  void iterate();
  void formatset();
  void formatnotset();
  void specialchars();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VarLTest);


//  Construction should leave an empty list:
//
void VarLTest::construct() {

  EQ(zero, m_pList->size());
  vector<string> formatted = m_pList->format();
  EQ(zero, formatted.size());
}


// Test that we can add to the list and the number of
// items in the list grows:

void VarLTest::add()
{
  m_pList->add("Test1");
  m_pList->add("test2");

  EQ((size_t)2, m_pList->size());
}

// Removing items should decrease the item count reported by size()

void VarLTest::remove()
{
  add();			// stocks the list.
  m_pList->remove("test2");
  EQ((size_t)1, m_pList->size());
  m_pList->remove("Test1");
  EQ(zero, m_pList->size());

}
// Should be able to locate items that are in the list and not locate those
// not in the list:

void VarLTest::find()
{
  add();			// Stock the list.
  CVarList::VariableIterator p = m_pList->find("test2");
  ASSERT(p != m_pList->end());
  EQ(string("test2"), p->first);

  p = m_pList->find("george");
			   
  ASSERT(p == m_pList->end());
}
// Should be able to iterate through the list
//
void VarLTest::iterate()
{
  add();			// Stock the list.

  CVarList::VariableIterator p = m_pList->begin(); // points tothe first one.
  ASSERT(p != m_pList->end());
  p++;				// points to the second one.
  ASSERT (p != m_pList->end());
  p++;				// should be end.

  ASSERT(p == m_pList->end());
}
//  Formatting a variable that's not set should give a single vector element:
//  set testing {--Not Set--}
//
void VarLTest::formatnotset()
{
  m_pList->add("testing");
  vector<string> values = m_pList->format();
  EQ((size_t)1, values.size());
  EQ(string("set testing {--Not Set--}"), values[0]);
}
//
// Given a nice value like george format should give a single lement with
// set testing george
//
void VarLTest::formatset()
{
  m_pList->add("testing");
  m_pInterp->Eval("set testing george");
  vector<string> values = m_pList->format();
  EQ(string("set testing george"), values[0]);

}
// test formatting in the precense of special chars like [] and $

void VarLTest::specialchars()
{
  m_pList->add("testing");
  CTCLVariable t(m_pInterp, "testing", TCLPLUS::kfFALSE);
  t.Set("$abc[testig]");
  vector<string> values = m_pList->format();
  EQ(string("set testing {$abc[testig]}"), values[0]);
}
