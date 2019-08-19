/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CZMQTransformerApp.cpp
 *  @brief: Implement the transformer app with ZMQ messaging/thread parallelism.
 */

#include "CZMQTransformerApp.h"
#include "transformer.h"
#include "CRingItemZMQSourceElement.h"
#include "CThreadedProcessingElement.h"
#include "CZMQCommunicatorFactory.h"
#include "CTransport.h"
#include "CRingItemTransport.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CRingItemSorter.h"
#include "CDataSinkElement.h"
#include "CRingItemTransportFactory.h"
#include "CZMQDealerTransport.h"
#include "CRingBlockDataSink.h"
#include "CNullTransport.h"
#include "CBuiltRingItemExtender.h"


#include <stdlib.h>
#include <stdexcept>
#include <errno.h>

// Service numbers in the ZMQ transport file.

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);

/**
 * constructor
 *   @param args - references the parsed parameters struct.
 */
CZMQTransformerApp::CZMQTransformerApp(gengetopt_args_info& args) :
    CTransformerApp(args),
      m_pSourceElement(nullptr), m_pSourceThread(nullptr),
    
    m_pSortServer(nullptr), m_pSortReceiver(nullptr), m_pSortSource(nullptr),
    m_pSortSender(nullptr), m_pSortElement(nullptr), m_pSortThread(nullptr),
    
    m_pSortClient(nullptr), m_pSortData(nullptr),
    m_pRingSink(nullptr), m_pRingSender(nullptr),
    m_pSinkElement(nullptr), m_pSinkThread(nullptr)
    
{}

/**
 *  destructor
 *     Kill off all the objects.
 */
CZMQTransformerApp::~CZMQTransformerApp()
{
    delete m_pSourceThread;
    delete m_pSourceElement;
    
    
    delete m_pSortThread;
    delete m_pSortElement;          // Deletes the CSender & CReceiver.
    delete m_pSortSource;
    delete m_pSortServer;
    
    delete m_pSinkThread;
    delete m_pSinkElement;
    delete m_pSortClient;
    delete m_pRingSink;
    
    for(int i =0; i < m_workers.size(); i++) {
        delete m_workers[i];
    }
        
}
/**
 * operator()
 *    Runs the applications.
 *    Sets up and starts all of the threads:
 */
int
CZMQTransformerApp::operator()()
{
    // Create the data source object and encapsulate it in a thread:
    // Note that since the router is a req/rep style deal it's not
    // going to start sending data until there's at least one worker.
    
    CZMQCommunicatorFactory commFactory;          // URL translation.
    std::string routerUri = commFactory.getUri(DISTRIBUTION_SERVICE);
    m_pSourceElement =
        new CRingItemZMQSourceElement(
            m_params.source_arg, routerUri.c_str(), m_params.clump_size_arg
        );
    m_pSourceThread = new CThreadedProcessingElement(m_pSourceElement);
                      // Can start the thread.
    m_pSourceThread->start();
    
    // The next server we need to establish is the sorter.
    // The sorter is a pull server for fanin and a push server for
    // one to one.
    
    m_pSortServer = commFactory.createFanInSink(SORT_SERVICE);
    m_pSortReceiver = new CReceiver(*m_pSortServer);
    m_pSortSource   = commFactory.createOneToOneSource(SORTEDDATA_SERVICE);
    m_pSortSender   = new CSender(*m_pSortSource);
    m_pSortElement  = new CRingItemSorter(
        *m_pSortReceiver, *m_pSortSender, 0, // sort window arg is obsolete.
        m_params.workers_arg
    );
    m_pSortThread = new CThreadedProcessingElement(m_pSortElement);
    m_pSortThread->start();
    
    //  Create the ultimate data sink.  Gets data from the
    // SORTEDDATA_SERVICE and disposes it as determined by the --sink
    // command parameter:
    
  
    m_pSortClient   = commFactory.createOneToOneSink(SORTEDDATA_SERVICE);
    m_pSortData     = new CReceiver(*m_pSortClient);

    m_pRingSink     =
        CRingItemTransportFactory::createTransport(
            m_params.sink_arg, CRingBuffer::producer
        );
    m_pRingSender = new CSender(*m_pRingSink);
    m_pSinkElement = new CRingBlockDataSink(*m_pSortData, *m_pRingSender);
    m_pSinkThread  = new CThreadedProcessingElement(m_pSinkElement);
    m_pSinkThread->start();

 
    
    sleep(1);
    
    startWorkers();
    
    m_pSourceThread->join();
    m_pSortThread->join();
    m_pSinkThread->join();
    for (int i =0; i < m_workers.size(); i++) {
        m_workers[i]->join();
    }
    
   
    return EXIT_SUCCESS;
    
}
/**
 * startworkers
 *    Start the individual worker bees.
 *    - Get the extender factory from the dll,
 *    - Create worker threads that encaspulate a CRingBuiltItemExtender
 *      that use the user's class.
 *    - Thread object pointers are stored in m_workers and started.
 */
void
CZMQTransformerApp::startWorkers()
{
    CZMQCommunicatorFactory commFactory;
    ExtenderFactory fact = getExtenderFactory();
    for (int i =0; i < m_params.workers_arg; i++) {
        CBuiltRingItemExtender::CRingItemExtender* pExtender = (*fact)();
        std::string dealerUri = commFactory.getUri(DISTRIBUTION_SERVICE);
        CFanoutClientTransport *pFanoutClient =
            new CZMQDealerTransport(dealerUri.c_str());
        CTransport* pFaninXport =
            commFactory.createFanInSource(SORT_SERVICE);

        CSender*    pFaninSender = new CSender(*pFaninXport);
        CBuiltRingItemExtender* pWorker =
            new CBuiltRingItemExtender(*pFanoutClient, *pFaninSender, i+1, pExtender);
        CThreadedProcessingElement* pThread =
            new CThreadedProcessingElement(pWorker);
        pThread->start();
        m_workers.push_back(pThread);
    }
}    