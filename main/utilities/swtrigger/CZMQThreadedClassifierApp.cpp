/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file   CZMQThreadedClassifierApp.cpp
* @brief Implements the CZMQThreadedClassifierAPp class.
*/
#include "CZMQThreadedClassifierApp.h"
#include "CRingItemZMQSourceElement.h"
#include "CThreadedProcessingElement.h"
#include "CZMQCommunicatorFactory.h"
#include "CTransport.h"
#include "CSender.h"
#include "CReceiver.h"


#include <stdlib.h>

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);

/**
 * constructor
 *   @param args -the parsed arguments.
 */
CZMQThreadedClassifierApp::CZMQThreadedClassifierApp(gengetopt_args_info& args) :
    CClassifierApp(args),
    m_pSourceElement(nullptr), m_pSourceThread(nullptr),
    m_pSortServer(nullptr), m_pSortReceiver(nullptr), m_pSortSource(nullptr),
    m_pSortSender(nullptr), m_pSortElement(nullptr), m_pSortThread(nullptr)
{}
    
/**
 * destructor
 */
CZMQThreadedClassifierApp::~CZMQThreadedClassifierApp()
{
    delete m_pSourceThread;
    delete m_pSourceElement;
    
    delete m_pSortThread;
    //delete m_pSortELement;
    delete m_pSortSender;
    delete m_pSortSource;
    delete m_pSortReceiver;
    delete m_pSortServer;
}

/**
 * operator()
 *    Runs the application.
 * @return - the status to return on application exit.
 */
int
CZMQThreadedClassifierApp::operator()()
{
    // Create the data source object and encapsulate it in a thread:
    // Note that since the router is a req/rep style deal it's not
    // going to start sending data until there's at least one worker.
    
    CZMQCommunicatorFactory commFactory;          // URL translation.
    std::string routerUri = commFactory.getUri(DISTRIBUTION_SERVICE);
    m_pSourceElement =
        new CRingItemZMQSourceElement(m_params.source_arg, routerUri.c_str());
    m_pSourceThread = new CThreadedProcessingElement(m_pSourceElement);
    m_pSourceThread->start();                    // Can start the thread.
    
    // The next server we need to establish is the sorter.
    // The sorter is a pull server for fanin and a push server for
    // one to one.
    
    m_pSortServer = commFactory.createFanInSink(SORT_SERVICE);
    m_pSortReceiver = new CReceiver(*m_pSortServer);
    m_pSortSource   = commFactory.createOneToOneSource(SORTEDDATA_SERVICE);
    m_pSortSender   = new CSender(*m_pSortSource);
    // Create the sorting object.
    // wrap it in a thread.
    // start the thread.
    
    sleep(1);                    // Let the threads get established.
    
    // Wait for them all to end:
    
    m_pSourceThread->join();
    return EXIT_SUCCESS;
}