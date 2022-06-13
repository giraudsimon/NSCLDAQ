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

/** @file:  CZMQAppStrategy.h
 *  @brief: Strategy class for ZMQ threaded event processing.
 */
#ifndef CZMQAPPSTRATEGY_H
#define CZMQAPPSTRATEGY_H
// #include "CRingItemMarkingWorker.h"
#include "swtriggerflags.h"

#include <vector>

class CRingItemZMQSourceElement;
class CThreadedProcessingElement;


class CTransport;
class CSender;
class CReceiver;
class CRingItemSorter;

class CDataSinkElement;

/**
 * @class CZMQAppStrategy
 *     The executable programs created that support ZMQ threading
 *     all have the same general structure.  An input rhead reads
 *     data from some source and fans data out to some set of workers.
 *     The works fan in their results to a timestamp sorter thread
 *     that, in turn, pipes the data to an output thread which
 *     stores the data in some data sink.
 *
 *     The application dependent part of this is the actual worker
 *     objects in the worker threads.   This class is a pure virtual
 *     base class which only leaves the concrete class the
 *     job of creating a CThreadedProcessingElement for each
 *     worker thread on demand given the transports and consumer
 *     id for the thread.
 *
 *     Actual applications can incorporate their version of this
 *     class and leverage the work this class does of setting up
 *     the standard threads and the communication endpoints
 *     used by all threads.  See e.g. CMPIAppStrategy for the same
 *     deal for MPI distributed processing.
 */
class CZMQAppStrategy
{
private:
    // member data:
    
    // Data source objects.
    
    CRingItemZMQSourceElement* m_pSourceElement;
    CThreadedProcessingElement* m_pSourceThread;
    
    // Stuff needed to support the sorter.
    
    CTransport*                 m_pSortServer;
    CReceiver*                  m_pSortReceiver;
    CTransport*                 m_pSortSource;
    CSender*                    m_pSortSender;
    CRingItemSorter*            m_pSortElement;
    CThreadedProcessingElement* m_pSortThread;
    
    // Stuff for the ultimate data sink:
    
    CTransport*                 m_pSortClient;
    CReceiver*                  m_pSortData;
    CTransport*                 m_pRingSink;
    CSender*                    m_pRingSender;
    CDataSinkElement*           m_pSinkElement;
    CThreadedProcessingElement* m_pSinkThread;
    
    std::vector<CThreadedProcessingElement*> m_workers;
    
    gengetopt_args_info& m_params;
public:
    CZMQAppStrategy(gengetopt_args_info& args);
    virtual ~CZMQAppStrategy();
    
    virtual int operator()();
protected:
    virtual void startWorkers();
    virtual CThreadedProcessingElement* makeWorker(
        CTransport& source,
        CSender&    sink,
        int         id
    ) = 0;
};

#endif