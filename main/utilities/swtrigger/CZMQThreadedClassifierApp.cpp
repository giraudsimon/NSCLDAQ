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
#include "CRingItemTransport.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CRingItemSorter.h"
#include "CDataSinkElement.h"
#include "CRingItemTransportFactory.h"
#include "CZMQDealerTransport.h"
#include "CRingBlockDataSink.h"
#include "CNullTransport.h"
#include "CZMQAppStrategy.h"


#include <stdlib.h>
#include <stdexcept>
#include <errno.h>

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);


/////////////// Our strategy class specialised to make our workers

class CZMQClassifierStrategy : public CZMQAppStrategy
{
private:
    CClassifierApp::ClassifierFactory m_fact;
public:
    CZMQClassifierStrategy(
        gengetopt_args_info& args,
        CClassifierApp::ClassifierFactory fact
    );
    virtual ~CZMQClassifierStrategy() {}
    
    virtual CThreadedProcessingElement* makeWorker(
        CTransport& source,
        CSender&    sink,
        int         id
    );
};
/**
 * constructor
 *   Just init the base class and save the factory
 * @param args - parsed arguments.
 * @param fact - User's classifier factory.
 */
CZMQClassifierStrategy::CZMQClassifierStrategy(
    gengetopt_args_info& args, CClassifierApp::ClassifierFactory fact
) :
    CZMQAppStrategy(args),
    m_fact(fact)
{}
/**
 * Create a worker thread element.
 *   @param source - transport that carries the data source.
 *   @param sink   - sender for the sink.
 *   @param id     - client id for the fanout source.
 */
CThreadedProcessingElement*
CZMQClassifierStrategy::makeWorker(
    CTransport& source,
    CSender&    sink,
    int         id
)
{
    CRingMarkingWorker::Classifier* pClassifier = (*m_fact)();
    CRingMarkingWorker* pWorker = new CRingMarkingWorker(
        dynamic_cast<CFanoutClientTransport&>(source), sink, id,
        pClassifier
    );
    return new CThreadedProcessingElement(pWorker);
}

/**
 * constructor
 *   @param args -the parsed arguments.
 *   @param fact - the factory.
 */
CZMQThreadedClassifierApp::CZMQThreadedClassifierApp(gengetopt_args_info& args) :
    CClassifierApp(args),
    m_strategy(nullptr)
    
    
{
    m_strategy =
        new CZMQClassifierStrategy(args, getClassifierFactory());        
}
    
/**
 * destructor
 */
CZMQThreadedClassifierApp::~CZMQThreadedClassifierApp()
{
    delete m_strategy;
}

/**
 * operator()
 *    Runs the application.
 * @return - the status to return on application exit.
 */
int
CZMQThreadedClassifierApp::operator()()
{
    return (*m_strategy)();
}
