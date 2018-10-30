// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CMonVar.h"
#undef private

#include <stdexcept>


class MonVarDictTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MonVarDictTests);
  CPPUNIT_TEST(oneadd);
  CPPUNIT_TEST(sortedadd);
  CPPUNIT_TEST(dupadd);
  
  CPPUNIT_TEST(remove);
  CPPUNIT_TEST(removenox);
  
  CPPUNIT_TEST(iterate);
  CPPUNIT_TEST_SUITE_END();


private:
  CMonvarDictionary* m_pDict;
public:
  void setUp() {
    m_pDict = CMonvarDictionary::getInstance();
  }
  void tearDown() {
    delete CMonvarDictionary::m_pInstance;  // Force new creation of
    CMonvarDictionary::m_pInstance =0;      // singleton each time.
  }
protected:
  void oneadd();
  void sortedadd();
  void dupadd();
  
  void remove();
  void removenox();
  
  void iterate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MonVarDictTests);

// first an empty list, then adding one gives one item:

void MonVarDictTests::oneadd() {
    EQ(size_t(0), m_pDict->get().size());
    
    std::string el("FirstOne");
    m_pDict->add(el);
    const std::list<std::string>& l(m_pDict->get());
    
    EQ(size_t(1) , l.size());
    EQ(el, l.front());
  
}
// additions remain sorted:

void MonVarDictTests::sortedadd()
{
  oneadd();                  // Now we have a list with "FirstOne"
  std::string s("ASecondOne");
  m_pDict->add(s);           // Should be first.
  
  const std::list<std::string>& l(m_pDict->get());
  
  EQ(size_t(2) , l.size());
  EQ(s, l.front());

}
// Duplicate addition throws std:logic_error:

void MonVarDictTests::dupadd()
{
  m_pDict->add("Item1");
  CPPUNIT_ASSERT_THROW(
    m_pDict->add("Item1"),
    std::logic_error
  );
}

// can remove entries:
void MonVarDictTests::remove()
{
  m_pDict->add("Item1");
  m_pDict->add("WillBeRemoved");
  m_pDict->add("Item2");
  
  m_pDict->remove("WillBeRemoved");
  
  const std::list<std::string>& l(m_pDict->get());
  EQ(size_t(2), l.size());
  EQ(std::string("Item1"), l.front());
  EQ(std::string("Item2"), l.back());
  
  
}
// throws logic error if removing a nonexistent element.

void MonVarDictTests::removenox()
{
  m_pDict->add("Item1");
  m_pDict->add("Item2");
  m_pDict->add("Item3");
  
  CPPUNIT_ASSERT_THROW(
    m_pDict->remove("NoSuchItem"),
    std::logic_error
  );
}

// The iterations works too:

void MonVarDictTests::iterate()
{
  m_pDict->add("Item1");
  m_pDict->add("Item2");
  m_pDict->add("Item3");

  std::list<std::string>::iterator p = m_pDict->begin();
  EQ(std::string("Item1"), *p);
  p++;
  EQ(std::string("Item2"), *p);
  p++;
  EQ(std::string("Item3"), *p);
  p++;
  ASSERT(p == m_pDict->end());
}