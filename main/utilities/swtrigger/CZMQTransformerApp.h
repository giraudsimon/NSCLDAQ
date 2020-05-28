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

/** @file:  CZMQTransformerApp.h
 *  @brief: Header for a transformer application that sets up
 *          for ZMQ/threaded parallelization strategy.
 */
#ifndef CZMQTRANSFORMERAPP_H
#define CZMQTRANSFORMERAPP_H
#include "CTransformerApp.h"
#include "CBuiltRingItemExtender.h"

#include <vector>

class CRingItemZMQSourceElement;
class CThreadedProcessingElement;

class CTransport;
class CSender;
class CReceiver;
class CRingItemSorter;

class CDataSinkElement;
class CZMQAppStrategy;

/**
 * @class CZMQTransformerApp
 *     This is a concrete class that uses ZMQ for communications and
 *     threaded parallelism that runs inside a specific system.  To make
 *     use of this class th euser must have created configuration files that defi
ne
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
class CZMQTransformerApp : public CTransformerApp
{
private:
    CZMQAppStrategy* m_strategy;
    
public:
    CZMQTransformerApp(gengetopt_args_info& args);
    virtual ~CZMQTransformerApp();
    virtual int operator()();

};

#endif