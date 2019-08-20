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

/** @file:  CZMQBuiltRingItemEditorApp.cpp
 *  @brief: Implement the ZMQ based ring item editor..
 */

#include "CZMQBuiltRingItemEditorApp.h"
#include "eventeditor.h"


#include "CBuiltRingItemEditor.h"

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
#include "CBuiltRingItemEditor.h"


#include <stdlib.h>
#include <stdexcept>
#include <errno.h>


// Service numbers in the ZMQ transport file.

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);

/**
 * constructor
 *    Just initializes all the object pointer to null.
 *    
 * @param args - the parsed parameters.
 *
 */
CZMQBuiltRingItemEditorApp::CZMQBuiltRingItemEditorApp(gengetopt_args_info args) :
    CBuiltRingItemEditorApp(args),
    m_pSourceElement(nullptr), m_pSourceThread(nullptr),
    
    m_pSortServer(nullptr), m_pSortReceiver(nullptr), m_pSortSource(nullptr),
    m_pSortSender(nullptr), m_pSortElement(nullptr), m_pSortThread(nullptr),
    
    m_pSortClient(nullptr), m_pSortData(nullptr),
    m_pRingSink(nullptr), m_pRingSender(nullptr),
    m_pSinkElement(nullptr), m_pSinkThread(nullptr)
    {}
    

/**
 * destructor
 *    - Must be called after everything has been joined.
 *    - Deletes the objects in the proper order.
 */
CZMQBuiltRingItemEditorApp::~CZMQBuiltRingItemEditorApp()
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
 *    Sets up the application objects.
 *    Starts the workers
 *    Waits for everything to finish.
 */
void
CZMQBuiltRingItemEditorApp::operator()()
{
    setupApplication();
    sleep(1);              // Let everything get settled in.
    setupWorkers();
    
    m_pSourceThread->join();
    m_pSortThread->join();
    m_pSinkThread->join();
    for (int i =0; i < m_workers.size(); i++) {
        m_workers[i]->join();
    }
}
/**
 * setupApplication
 *   Starts the scaffolding threads of the application.
 *   This is the data source, the sorter and the sink:
 */
void
CZMQBuiltRingItemEditorApp::setupApplication()
{
    // Create the data source object and encapsulate it in a thread:
    // Note that since the router is a req/rep style deal it's not
    // going to start sending data until there's at least one worker.
    
    CZMQCommunicatorFactory commFactory;          // URL translation.
    std::string routerUri = commFactory.getUri(DISTRIBUTION_SERVICE);
    m_pSourceElement =
        new CRingItemZMQSourceElement(
            m_args.source_arg, routerUri.c_str(), m_args.clump_size_arg
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
        m_args.workers_arg
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
            m_args.sink_arg, CRingBuffer::producer
        );
    m_pRingSender = new CSender(*m_pRingSink);
    m_pSinkElement = new CRingBlockDataSink(*m_pSortData, *m_pRingSender);
    m_pSinkThread  = new CThreadedProcessingElement(m_pSinkElement);
    m_pSinkThread->start();
    
}
/**
 * startWorkers
 */
void
CZMQBuiltRingItemEditorApp::setupWorkers()
{
    CZMQCommunicatorFactory commFactory;
    EditorFactory fact = getEditorFactory();
    for (int i =0; i < m_args.workers_arg; i++) {
        CBuiltRingItemEditor::BodyEditor* pEditor = (*fact)();
        std::string dealerUri = commFactory.getUri(DISTRIBUTION_SERVICE);
        CFanoutClientTransport *pFanoutClient =
            new CZMQDealerTransport(dealerUri.c_str());
        CTransport* pFaninXport =
            commFactory.createFanInSource(SORT_SERVICE);

        CSender*    pFaninSender = new CSender(*pFaninXport);
        CBuiltRingItemEditor* pWorker =
            new CBuiltRingItemEditor(*pFanoutClient, *pFaninSender, i+1, pEditor);
        CThreadedProcessingElement* pThread =
            new CThreadedProcessingElement(pWorker);
        pThread->start();
        m_workers.push_back(pThread);
    }
}