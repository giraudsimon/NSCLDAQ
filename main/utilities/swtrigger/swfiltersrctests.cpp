// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "swFilterRingDataSource.h"
#include <CDataSource.h>
#include <CRingItem.h>

#include <list>

// mock for CDataSource

class CFakeDataSource : public CDataSource
{
private:
  std::list<CRingItem*> m_items;
public:
  virtual ~CFakeDataSource() {}
  void addItem(CRingItem* pItem);    // Must have been new'd.
  CRingItem* getItem() {
    CRingItem* result(nullptr);    // result if there's nothing left.
    
    if(!m_items.empty()) {         // but if there is...
      result = m_items.front();
      m_items.pop_front();
    }
    
    return result;
  }
  void read(char* pBuffer, size_t nBytes) {}
};


class ringsourceTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ringsourceTests);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CFakeDataSource*        m_pSrc;
  swFilterRingDataSource* m_pTestObject;
public:
  void setUp() {
    m_pSrc = new CFakeDataSource;
    m_pTestObject = new swFilterRingDataSource(*m_pSrc);
  }
  void tearDown() {
    delete m_pTestObject;
    delete m_pSrc;
    
    m_pTestObject = nullptr;
    m_pSrc        = nullptr;
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ringsourceTests);

void ringsourceTests::aTest() {
}
