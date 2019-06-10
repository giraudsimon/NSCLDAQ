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
    size_t nInts = nBytes/sizeof(int);
    
    std::sort(p, p+nInts);
  }
  CSender* pSink = getSink();
  pSink->sendMessage(p, nBytes);           // Sort was in place.
  usleep(1000);                            // avoid early termination of puller
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


class pworkerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(pworkerTest);
  CPPUNIT_TEST(work_1);
  CPPUNIT_TEST(work_2);
  CPPUNIT_TEST_SUITE_END();


private:
  // For the data source
  CTestTransport*      m_pTestData;
  CReceiver*           m_pTestReceiver;
  CZMQRouterTransport* m_pRouter;
  CDataSourceElement*  m_pSrc;
  
  // For the worker:
  
  CZMQDealerTransport* m_pWorkerReceiver;
  CZMQClientTransport*    m_pWorkerOutputTransport;
  CSender*             m_pWorkerSender;
  ParallelSorter*      m_pWorker;
  
  // For the receipient of all of this:
  
  CZMQServerTransport*    m_pPuller;
  
  
public:
  void setUp() {       //Note the tests start/join the threads.
    
    // The data source -- fill test transport before running.
    
    m_pTestData = new CTestTransport;
    m_pTestReceiver = new CReceiver(*m_pTestData);
    m_pRouter       = new CZMQRouterTransport(fanoutservice.c_str());
    m_pSrc          = new CDataSourceElement(*m_pTestReceiver, *m_pRouter);
    // So we can get results:
    
    m_pPuller = new CZMQServerTransport(faninservice.c_str(), ZMQ_PULL);
    usleep(100);    
    // One sort worker:
    
    m_pWorkerReceiver = new CZMQDealerTransport(fanoutservice.c_str(), 1);
    m_pWorkerOutputTransport = new CZMQClientTransport(faninservice.c_str(), ZMQ_PUSH);
    m_pWorkerSender  = new CSender(*m_pWorkerOutputTransport);
    m_pWorker        = new ParallelSorter(*m_pWorkerReceiver, *m_pWorkerSender);
    
    
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
  void work_1();
  void work_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(pworkerTest);

void pworkerTest::work_1() {   // Handle 1 work item.
  // Start the worker and receiver threads:
  
  std::thread worker(std::ref(*m_pWorker));
  TestReceiver r(m_pPuller);
  std::thread receiver(std::ref(std::ref(r)));
  
  // Create a bit of test data unsorted and add it.
  
  int numbers[100];
  for (int i = 0; i < sizeof(numbers)/sizeof(int); i++) {
    numbers[i] = random() % 100;            // Rang of numbers is 0-99.
  }
  
  m_pTestData->addMessage(&numbers, sizeof(numbers));
  
  // Run the data source after processing the data, both thread should exit.
  
  (*m_pSrc)();
  
  
  worker.join();
  receiver.join();
  
  // receiver should have the sorted data:

  EQ(size_t(2), r.m_received.size());
  EQ(sizeof(numbers), r.m_received[0].first);
  EQ(size_t(0), r.m_received[1].first);
  
  // Compare the data in the first msg with the sorted data array:
  
  int* p = static_cast<int*>(r.m_received[0].second);
  std::sort(numbers, numbers+100);
  for (int i =0; i < 100; i++) {
    EQ(numbers[i], p[i]);
  }
  
  
}

void pworkerTest::work_2()   // 2 workers in parallel.
{
  // Make the second worker:
  
  CZMQDealerTransport w2receiver(fanoutservice.c_str(), 2);
  CZMQClientTransport w2sendert(faninservice.c_str(), ZMQ_PUSH);
  CSender*            w2sender = new CSender(w2sendert); // gets deleted.
  ParallelSorter     w2(w2receiver, *w2sender);
  
  // start the two worker threads and the puller:
  
  TestReceiver r(m_pPuller);
  std::thread receiver(std::ref(std::ref(r)));
  std::thread worker1(std::ref(*m_pWorker));
  std::thread worker2(std::ref(w2));
  
  // Put two work items in the test data source.. they'll be
  // disjoint so we can tell the sorted versions apart:
  
  int workItem1[100];             // 0-99
  int workItem2[100];             // 100-199.
  for (int i =0; i < 100; i++) {
    workItem1[i] = random() % 100;
    workItem2[i] = 100 + (random() % 100);
  }
  // Put the work items in to the test data source:
  
  m_pTestData->addMessage(workItem1, sizeof(workItem1));
  m_pTestData->addMessage(workItem2, sizeof(workItem2));
  
  // Run the sender -- both workers should end (I think), Due to
  // balanced distribution of work items...as should the receiver.
  
  (*m_pSrc)();             // run the application.
  
  worker1.join();
  worker2.join();
  receiver.join();               // Actually exits on first end item.
  
  // The workers operate in parallel and sort times may vary so we don't
  // actually know the order in which they've output their data.
  
  EQ(size_t(3), r.m_received.size());
  EQ(sizeof(workItem1), r.m_received[0].first);  // works b/c equally sized
  EQ(sizeof(workItem1), r.m_received[1].first);  // work items.
  EQ(size_t(0), r.m_received[2].first);
 
  // Sort the input items for comparison.
 
  std::sort(workItem1, workItem1+100);
  std::sort(workItem2, workItem2+100);
 
  for (int w = 0; w < 2; w++) {                    // iterate over work items.
    int* p = static_cast<int*>(r.m_received[w].second);
    
    // Ah but which one -- we'll assume the sort worked and
    // look for a match in the first element.
    
    int* p1 = workItem1;
    if (*p != *p1) {
      p1 = workItem2;
    }
    
    for (int i =0; i < 100;i ++) {
      EQ(p1[i], p[i]);
    }
  }
}