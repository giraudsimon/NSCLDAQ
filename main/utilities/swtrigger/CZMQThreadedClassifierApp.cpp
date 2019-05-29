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
    m_pSourceElement(nullptr), m_pSourceThread(nullptr)
{}
    
/**
 * destructor
 */
CZMQThreadedClassifierApp::~CZMQThreadedClassifierApp()
{}

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
    std::string routerUri = commFactory.getUri(1);
    m_pSourceElement =
        new CRingItemZMQSourceElement(m_params.source_arg, routerUri.c_str());
    m_pSourceThread = new CThreadedProcessingElement(m_pSourceElement);
    m_pSourceThread->start();                    // Can start the thread.
    
    
    sleep(1);                    // Let the threads get established.
    
    // Wait for them all to end:
    
    m_pSourceThread->join();
    return EXIT_SUCCESS;
}