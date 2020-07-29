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
* @file   CZMQThreadedClassifierApp.h
* @brief  Concrete classifier app class using ZMQ and threads.
*/
#ifndef CZMQTHREADEDCLASSIFIERAPP_H
#define CZMQTHREADEDCLASSIFIERAPP_H
#include "CClassifierApp.h"
#include "CRingItemMarkingWorker.h"

#include <vector>

class CRingItemZMQSourceElement;
class CThreadedProcessingElement;

class CTransport;
class CSender;
class CReceiver;
class CRingItemSorter;
class CZMQAppStrategy;

class CDataSinkElement;

/**
 * @class CZMQThreadedClassifierApp
 *     This is a concrete class that uses ZMQ for communications and
 *     threaded parallelism that runs inside a specific system.  To make
 *     use of this class th euser must have created configuration files that define
 *     inproc URIs for:
 *     1   - The router of work items to  worker threads.
 *     2   - The push/pull fan-in from workers to timestamp sorter thread.
 *     3   - The push-pull one-to-one from sorter thread to the output thread.
 *
 *  With one worker, the application still has a pipeline parallelism of:
 *     data source -> worker -> timestamp-sortgin -> outputting
 *
 *  Though the sort thread won't need to do much as its data will come in sorted.
 *  Multiple workers simply fanout as a ROUTER/DEALER from source to workers and
 *  fan in using PUSH/PULL from workers to the timestamp sorter then pipeline
 *  parallelism again to the output thread.
 */
class CZMQThreadedClassifierApp : public CClassifierApp
{    
private:
    
    CZMQAppStrategy*  m_strategy;
    
public:
    CZMQThreadedClassifierApp(gengetopt_args_info& args);
    virtual ~CZMQThreadedClassifierApp();
    virtual int operator()();

};

#endif