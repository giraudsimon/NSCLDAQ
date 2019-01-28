// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "fakedatasource.h"
#include "ZMQPushDataSource.h"
#include "swFilterRingBlockDataSource.h"
#include "RingItemBlockConverter.h"

#include <DataFormat.h>
#include <CPhysicsEventItem.h>


#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <zmq.hpp>


static const std::string uri("inproc://test");


class zmqpushblocktest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqpushblocktest);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CFakeRingBlockReader*   m_pDataSource;
  RingItemBlockConverter* m_pConverter;
  swFilterRingBlockDataSource* m_pSrc;
  ZMQPushDataSource*      m_pTestObj;
  
  zmq::socket_t*          m_pSocket;
public:
  void setUp() {
    m_pDataSource = new CFakeRingBlockReader;
    m_pConverter  = new RingItemBlockConverter;
    m_pSrc        = new swFilterRingBlockDataSource(*m_pDataSource, 256);
    m_pTestObj    = new ZMQPushDataSource("test", uri, m_pSrc, m_pConverter);
    
    m_pSocket = static_cast<zmq::socket_t*>(m_pTestObj->connectAsSink());
    usleep(1000*100);            // Let the connection work.
  }
  void tearDown() {
    m_pTestObj->closeSink(m_pSocket);
    
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

CPPUNIT_TEST_SUITE_REGISTRATION(zmqpushblocktest);

void zmqpushblocktest::aTest() {
}
