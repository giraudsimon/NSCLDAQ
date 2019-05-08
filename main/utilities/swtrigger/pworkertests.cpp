// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CDataSourceElement.h"
#include "CParallelWorker.h"
#include "CTestTransport.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CZMQRouterTransport.h"             // Fanout.
#include "CZMQDealerTransport.h"             // Fanout client.
#include "CZMQTransport.h"               // Output from workers.
#include "CZMQServerTransport.h"
#include "CZMQClientTransport.h"


#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <thread>
#include <functional>
#include <algorithm>
#include <zmq.hpp>


static const std::string fanoutservice("inproc://fanout");
static const std::string faninservice("inproc://fanin");

/**
 * Our test parallel workers will take messages consisting of
 * A set of integers, sort them and send the resulting sorted array
 * to the sink:
 */

class ParallelSorter : public CParallelWorker
{
public:
  ParallelSorter(CFanoutClientTransport& fanin, CSender& sink) :
    CParallelWorker(fanin, sink) {}
  virtual void process(void* pData, size_t nBytes);
};
void
ParallelSorter::process(void* pData, size_t nBytes)
{
  int* p = static_cast<int*>(pData);
  if (nBytes) {                           // Else it's an end. .just pass it on.
        // Make integer pointer.
    size_t nInts =- nBytes/sizeof(int);
    
    std::sort(p, p+nInts);
  }
  CSender* pSink = getSink();
  pSink->sendMessage(p, nBytes);           // Sort was in place.
  
}

// This class receives messages on an arbitrary transport and stores them.
// normally goes into a thread.

class TestReceiver
{
public:
  std::vector<std::pair<size_t, void*> > m_received;   // messages received.
  CTransport*                            m_pTransport;
  
  TestReceiver(CTransport* t) : m_pTransport(t) {}
  ~TestReceiver() {
    for (int i =0; i < m_received.size(); i++) {
      free(m_received[i].second);
    }
  }
  
  void operator()() {
    size_t nBytes;
    void*  pMessage;
    do {
      m_pTransport->recv(&pMessage, nBytes);
      std::pair<size_t, void*> item;
      item.first = nBytes;
      item.second = pMessage;
      m_received.push_back(item);
    } while (nBytes);
  }
  
};


class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  // For the data source
  CTestTransport*      m_pTestData;
  CReceiver*           m_pTestReceiver;
  CZMQRouterTransport* m_pRouter;
  CDataSourceElement*  m_pSrc;
  
  // For the worker:
  
  CZMQDealerTransport* m_pWorkerReceiver;
  CZMQServerTransport*    m_pWorkerOutputTransport;
  CSender*             m_pWorkerSender;
  ParallelSorter*      m_pWorker;
  
  // For the receipient of all of this:
  
  CZMQClientTransport*    m_pPuller;
  
  
public:
  void setUp() {       //Note the tests start/join the threads.
    
    // The data source -- fill test transport before running.
    
    m_pTestData = new CTestTransport;
    m_pTestReceiver = new CReceiver(*m_pTestData);
    m_pRouter       = new CZMQRouterTransport(fanoutservice.c_str());
    m_pSrc          = new CDataSourceElement(*m_pTestReceiver, *m_pRouter);
    
    // One sort worker:
    
    m_pWorkerReceiver = new CZMQDealerTransport(fanoutservice.c_str(), 1);
    m_pWorkerOutputTransport = new CZMQServerTransport(faninservice.c_str(), ZMQ_PUSH);
    m_pWorkerSender  = new CSender(*m_pWorkerOutputTransport);
    m_pWorker        = new ParallelSorter(*m_pWorkerReceiver, *m_pWorkerSender);
    
    // So we can get results:
    
    m_pPuller = new CZMQClientTransport(faninservice.c_str(), ZMQ_PUSH);
    
  }
  void tearDown() {
    
    delete m_pSrc;     // deletes sinks/sources
    delete m_pWorker;  // But not underlying transports so:
    
    delete m_pPuller;
    delete m_pWorkerOutputTransport;
    delete m_pWorkerReceiver;
    delete m_pRouter;
    delete m_pTestData;
    
    usleep(1000);            // Let server endpoints become inactive.
    
    
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
