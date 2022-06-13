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
#include "swtriggerflags.h"
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
#include "CZMQAppStrategy.h"

#include <stdlib.h>
#include <stdexcept>
#include <errno.h>

// Service numbers in the ZMQ transport file.

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);

/////////////////////// Our strategy class //////////////////

class CZMQTransformerStrategy : public CZMQAppStrategy
{
private:
    ExtenderFactory m_fact;
public:
    CZMQTransformerStrategy(gengetopt_args_info& args, ExtenderFactory fact);
    virtual ~CZMQTransformerStrategy() {}
    
    virtual CThreadedProcessingElement* makeWorker(
        CTransport& source,
        CSender&    sink,
        int         id
    );
};
/**
 * constructor
 *    Just initialize the base class and set the factory.
 * @param args -parsed arguments.
 * @param fact -The factory that cretes new application specific extenders.
*/
CZMQTransformerStrategy::CZMQTransformerStrategy(
    gengetopt_args_info& args, ExtenderFactory fact
) :
    CZMQAppStrategy(args),
    m_fact(fact)
{}
/**
 * makeWorker
 *    Create a worker thread specific to our application.
 * @param source - source of data (must be a CFanoutClientTransport).
 * @param sink   - where workers send resulting data
 * @param id     - fanout client id to use.
 * @return CThreadedProcessingElement* - the processing thread.
 */
CThreadedProcessingElement*
CZMQTransformerStrategy::makeWorker(
    CTransport& source,
    CSender&    sink,
    int         id
)
{
    CBuiltRingItemExtender::CRingItemExtender* pExtender = (*m_fact)();
    CBuiltRingItemExtender* pWorker = new CBuiltRingItemExtender(
        dynamic_cast<CFanoutClientTransport&>(source), sink,
        id, pExtender
    );
    return new CThreadedProcessingElement(pWorker);
}

/**
 * constructor
 *   @param args - references the parsed parameters struct.
 */
CZMQTransformerApp::CZMQTransformerApp(gengetopt_args_info& args) :
    CTransformerApp(args),
    m_strategy(nullptr)
    
{
    m_strategy = new CZMQTransformerStrategy(args, getExtenderFactory());
}

/**
 *  destructor
 *     Kill off all the objects.
 */
CZMQTransformerApp::~CZMQTransformerApp()
{
    delete m_strategy;
    
        
}
/**
 * operator()
 *    Runs the applications.
 *    Sets up and starts all of the threads:
 */
int
CZMQTransformerApp::operator()()
{
    return (*m_strategy)();
}    
