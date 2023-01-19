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
#include "swtriggerflags.h"

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
#include "CZMQAppStrategy.h"

#include <stdlib.h>
#include <stdexcept>
#include <errno.h>

// Service numbers in the ZMQ transport file.

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);

//////////////// My strategy class /////////////////////////

class CZMQBuiltEditorStrategy : public CZMQAppStrategy
{
private:
    EditorFactory m_fact;
public:
    CZMQBuiltEditorStrategy(
        gengetopt_args_info& args, EditorFactory fact
    );
    virtual ~CZMQBuiltEditorStrategy() {}
    virtual CThreadedProcessingElement* makeWorker(
        CTransport& source,
        CSender&    sink,
        int         id
    );
};
/**
 * constructor
 *    just initialize the base class and save the factory.
 * @param args - parsed arguments.
 * @param fact - edtro factory.
 */
CZMQBuiltEditorStrategy::CZMQBuiltEditorStrategy(
    gengetopt_args_info& args, EditorFactory fact
) :
    CZMQAppStrategy(args),
    m_fact(fact)
{}

/**
 * makeWorker
 *   Create the application specific worker thread.
 *   The worker code is encapsulated in the stuff needed
 *   to be a thread and to communicate with the rest of the
 *   application.
 * @param source - where the data comes from (must be a
 *                 CFanoutClientTransport)
 * @param sink   - where the produced data goes.
 * @param id     - id to use as the fanout client.
 * @return CThreadedProcessingElement* - the thread object.
 */
CThreadedProcessingElement*
CZMQBuiltEditorStrategy::makeWorker(
    CTransport& source,
    CSender&    sink,
    int         id
)
{
    CBuiltRingItemEditor::BodyEditor* pEditor = (*m_fact)();
    CBuiltRingItemEditor* pWorker = new CBuiltRingItemEditor(
        dynamic_cast<CFanoutClientTransport&>(source), sink, id, pEditor
    );
    return new CThreadedProcessingElement(pWorker);
}
/**
 * constructor
 *    Just initializes all the object pointer to null.
 *    
 * @param args - the parsed parameters.
 *
 */
CZMQBuiltRingItemEditorApp::CZMQBuiltRingItemEditorApp(gengetopt_args_info& args) :
    CBuiltRingItemEditorApp(args),
    m_strategy(nullptr)
{
  m_strategy = new CZMQBuiltEditorStrategy(args, getEditorFactory());
}

/**
 * destructor
 *    - Must be called after everything has been joined.
 *    - Deletes the objects in the proper order.
 */
CZMQBuiltRingItemEditorApp::~CZMQBuiltRingItemEditorApp()
{
    delete m_strategy;        
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
  (*m_strategy)();
}
