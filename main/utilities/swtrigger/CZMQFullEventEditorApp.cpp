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

/** @file:  CZMQFullEventEditorApp.cpp
 *  @brief: Implements the full event editor app with ZMQ transport.
 */
#include "CZMQFullEventEditorApp.h"
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
#include "CFullEventEditor.h"
#include "CZMQAppStrategy.h"


#include <stdlib.h>
#include <stdexcept>
#include <errno.h>



// Service numbers in the ZMQ transport file.

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);

//////////////////////// My strategy class////////////////////

class CZMQFullEditorStrategy : public CZMQAppStrategy
{
private:
 CZMQFullEventEditorApp* m_app;
public:
 CZMQFullEditorStrategy(gengetopt_args_info& args, CZMQFullEventEditorApp* app);
 virtual ~CZMQFullEditorStrategy() {}
 virtual CThreadedProcessingElement* makeWorker(
      CTransport& source,
      CSender&    sink,
      int         id
  );
};
/**
 * constructor
 *   Initialize the base class and save the application object
 *   which knows how to create application specific processing objects.
 * @param args - parsed arguments.
 * @param app  - pointer to the application.
 */
CZMQFullEditorStrategy::CZMQFullEditorStrategy(
 gengetopt_args_info& args, CZMQFullEventEditorApp* app
) :
 CZMQAppStrategy(args),
 m_app(app)
{}
/**
 * makeWorker
 *    Make a single worker thread.
 * @return CThreadedProcessingElement*
 */
CThreadedProcessingElement*
CZMQFullEditorStrategy::makeWorker(
      CTransport& source,
      CSender&    sink,
      int         id
  )
{
 CFullEventEditor::Editor* pEditor = m_app->createUserEditor();
 CFullEventEditor* pWorker = new CFullEventEditor(
    dynamic_cast<CFanoutClientTransport&>(source), sink, id, pEditor
 );
 return new CThreadedProcessingElement(pWorker);
}

/**
 * constructor
 *   @param args - reerences the parameters parsed by gengetopt.
 */
CZMQFullEventEditorApp::CZMQFullEventEditorApp(gengetopt_args_info& args) :
    CFullEventEditorApp(args),
    m_strategy(nullptr)
    
{
 m_strategy = new CZMQFullEditorStrategy(args, this);     
}
/**
 * destructor
 *    Destroys what we can in an appropriate order.  Note that in general
 *    the app lives for the lifetime of the program now so there's not
 *    a real + to doing this other than purity of essence.
 */
CZMQFullEventEditorApp::~CZMQFullEventEditorApp()
{
  delete m_strategy;
}
/**
 * operator()
 *   Sets up all of the communication and starts worker threads. Note that
 *   in the ZMQ transport world the code is threaded with ZMQ communication
 *   between the threads.  The service URI to service number maps are in the
 *   a zmq services file that the user has created.  See the documentation
 *   of CZMQCommunicatorFactory for information about the format of this file
 *   and how it is located.  The file must define a least the following
 *   service numbers:
 *
 *   - 1   - distributes events (Fanout) from source to workers.
 *   - 2   - Receives processed events for resorting in timestamp order.
 *   - 3   - Sends data from the sorter to the sink.
 *
 *   it is recommended these URIs be inproc URIs for example in testing
 *   I used:
 *   \verbatim
 *
1 inproc://distro
2 inproc://sort
3 inproc://sorted
*  \endverbatim
*
*     @return int - status - 0 means normal.
*     @throw may chose to fail by throwing exceptions instead.
*/
int
CZMQFullEventEditorApp::operator()()
{
  return (*m_strategy)();
}