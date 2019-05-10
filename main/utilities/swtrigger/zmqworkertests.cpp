// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CZMQRouterTransport.h"
#include "CZMQServerTransport.h"
#include "CZMQClientTransport.h"

#include "CRingItemDispatcher.h"
#include "CTestTransport.h"

#include "CZMQRingItemWorker.h"

#include "CProcessor.h"
#include "CSender.h"
#include "CReceiver.h"

#include <CFileDataSink.h>
#include <CRingItem.h>
#include <DataFormat.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/io.h>
#include <stdint.h>
#include <zmq.hpp>
#include <thread>
#include <functional>

static const std::string routeruri("inproc://router");
static const std::string sinkuri("inproc://sink");
static const std::string tmpFileTemplate("ringFileXXXXXX");


// Our app will count ring items in parallel.  Ring items will come from a
// temporary file.
//   This processor counts work items it receives until it gets an end
//   at which point it sends the number of work items it's seen to the sink.


// Inner processor.

class RingItemCounter : public CProcessor
{
  int   m_items;
public:
  RingItemCounter() : m_items(0) {}
  void process(void* pData, size_t nBytes, CSender& sender)
  {
      if (nBytes) {
      m_items++;
    } else {
      sender.sendMessage(&m_items, sizeof(m_items));
      sender.sendMessage((void*)(nullptr), 0);        // end of data.
    }
  }
};

// Data sink:

class CCountingSink : public CProcessingElement
{
public:
  int m_totalItems;
  int m_workersLeft;
  CReceiver& m_receiver;
public:
  CCountingSink(CReceiver& receiver, int nWorkers) :
    m_totalItems(0), m_workersLeft(nWorkers), m_receiver(receiver) {}
  void operator()();
  void process(void* pData, size_t nBytes);
  
};
void
CCountingSink::operator()() {
  while (m_workersLeft) {
    size_t nBytes;
    void*  pData;
    m_receiver.getMessage(&pData, nBytes);
    if (nBytes > 0) {
      process(pData, nBytes);
    } else {
      m_workersLeft--;
    }
    free (pData);
  }
}

void
CCountingSink::process(void* pData, size_t nBytes) {
  int* p = static_cast<int*>(pData);
  m_totalItems += *p;
}

class zmqworkerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zmqworkerTest);
  CPPUNIT_TEST(app_1);
  CPPUNIT_TEST_SUITE_END();


private:
  // The sink endpoint of the processes:
  
  CZMQServerTransport* m_pSinkSourceTransport;
  CReceiver*           m_pSinkReceiver;
  CCountingSink*       m_pSink;

  // The worker:
  
  
  RingItemCounter*     m_pCounter1;
  CZMQClientTransport* m_pCounterOut1;
  CSender*             m_pOut1;
  CZMQRingItemWorker*  m_pWorker1;
  
  
  RingItemCounter*     m_pCounter2;
  CZMQClientTransport* m_pCounterOut2;
  CSender*             m_pOut2;
  CZMQRingItemWorker*  m_pWorker2;
  
  // The actual source of data is a file containing ring items the test
  // stocks.
  
  int                  m_nRingFileId;
  std::string          m_RingFilename;
  CZMQRouterTransport* m_pSourceTransport;
  CSender*             m_pSourceSender;
  CRingItemDispatcher* m_pDataSource;
  
public:
  void setUp() {
    
    // Set up the ultimate data sink:
    
    m_pSinkSourceTransport = new CZMQServerTransport(sinkuri.c_str(), ZMQ_PULL);
    m_pSinkReceiver        = new CReceiver(*m_pSinkSourceTransport);
    m_pSink                = new CCountingSink(*m_pSinkReceiver, 2);  // 2 workers.
    
    // Set up the two workers:
    
    m_pCounter1           = new RingItemCounter;
    m_pCounterOut1        = new CZMQClientTransport(sinkuri.c_str(), ZMQ_PUSH);
    m_pOut1               = new CSender(*m_pCounterOut1);
    m_pWorker1            =
      new CZMQRingItemWorker(routeruri.c_str(), 1, *m_pOut1, m_pCounter1);
    
    m_pCounter2           = new RingItemCounter;
    m_pCounterOut2        = new CZMQClientTransport(sinkuri.c_str(), ZMQ_PUSH);
    m_pOut2               = new CSender(*m_pCounterOut2);
    m_pWorker2            =
      new CZMQRingItemWorker(routeruri.c_str(), 2, *m_pOut2, m_pCounter2);
    

    // The ultimate data source:
    
    char nameTemplate[tmpFileTemplate.size() +1];
    strcpy(nameTemplate, tmpFileTemplate.c_str());   // mkstemp needs modifiable str.
    m_nRingFileId  = mkstemp(nameTemplate);          /// an makes it the actual name.
    m_RingFilename = nameTemplate;
    std::string ringFileURI = "file://./";
    ringFileURI += m_RingFilename;
    
    m_pSourceTransport = new CZMQRouterTransport(routeruri.c_str());
    m_pSourceSender    = new CSender(*m_pSourceTransport);
    m_pDataSource      =
      new CRingItemDispatcher(ringFileURI.c_str(), m_pSourceSender);
    
  }
  void tearDown() {
    delete m_pSink;
    delete m_pSinkReceiver;
    delete m_pSinkSourceTransport;
    
    delete m_pWorker1;
    delete m_pOut1;
    delete m_pCounterOut1;
    delete m_pCounter1;
    
    delete m_pWorker2;
    delete m_pOut2;
    delete m_pCounterOut2;
    delete m_pCounter2;

    close(m_nRingFileId);
    unlink(m_RingFilename.c_str());
    delete m_pDataSource;
    delete m_pSourceSender;
    delete m_pSourceTransport;
    
    usleep(1000);
  }
protected:
  void app_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zmqworkerTest);

void zmqworkerTest::app_1() {
  // Given the fd, we can create a CFileDatasink and write a pile of
  // ring items to it.
  
  const int EVENTS = 10000;
  {
    CFileDataSink testData(m_nRingFileId);
    for (int i =0;  i < EVENTS; i++) {           // Make a lot of items.
      CRingItem item(PHYSICS_EVENT, i, 0, 0);   // With body header and all.
      uint32_t* pBody = static_cast<uint32_t*>(item.getBodyCursor());
      *pBody++ = i;
      item.setBodyCursor(pBody);
      item.updateSize();
      testData.putItem(item);
    }
  }                            // Sink is destroyed here.
  
  // Start up the threads:
  // We'll have a sink thread, and two worker threads.
  // The data source will get run in this thread.
  
  std::thread counter (std::ref(*m_pSink));
  std::thread worker1(std::ref(*m_pWorker1));
  std::thread worker2(std::ref(*m_pWorker2));
  
  // Run the source:
  
  class DataSource {
  private:
    CDispatcher* m_pDisp;
  public:
    DataSource(CDispatcher* pDisp) : m_pDisp(pDisp) {}
    void operator()() {
      void* pData;
      size_t n;
      do {
        m_pDisp->receiveWorkItem(&pData, n);
        
        if (n) m_pDisp->sendWorkItem(pData, n);
        free(pData);
      } while (n);
      m_pDisp->end();
    }
  };
  DataSource src(m_pDataSource);
  
  
  (src)();               // Process the file.
  
  // Join the threads:
  
  worker1.join();
  worker2.join();
  counter.join();
  
  
  EQ(EVENTS, m_pSink->m_totalItems);
  
  
}
