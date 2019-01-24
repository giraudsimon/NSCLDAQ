// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>


#include "fakedatasource.h"
#include "SingleRingItemConverter.h"
#include "swFilterRingDataSource.h"
#include "ZMQPushDataSource.h"

#include <zmq.hpp>


#include <string>

static const std::string uri="inproc://test";

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CFakeDataSource*         m_pDataSource;
  SingleRingItemConverter* m_pConverter;
  swFilterRingDataSource*  m_pSrc;
  ZMQPushDataSource*       m_pTestObj;
public:
  void setUp() {
    m_pDataSource = new CFakeDataSource;
    m_pConverter  = new SingleRingItemConverter;
    m_pSrc        = new swFilterRingDataSource(*m_pDataSource);
    m_pTestObj    = new ZMQPushDataSource("source", uri, m_pSrc, m_pConverter);
  }
  void tearDown() {
    // We assume the message get fully drained.
    
    delete m_pTestObj;
    delete m_pConverter;
    delete m_pSrc;
    delete m_pDataSource;
    
    m_pTestObj = nullptr;
    m_pConverter = nullptr;
    m_pSrc   = nullptr;
    m_pDataSource = nullptr;
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
