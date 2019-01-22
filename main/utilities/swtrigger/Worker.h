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

/** @file:  Worker.h
 *  @brief: This is an abstract base class for a software trigger worker.
 */

#ifndef WORKER_H
#define WORKER_H
#include "Thread.h"
#include <stdint.h>
#include <list>

/**
 * @class CWorker
 *     This class is the abstract base class of a worker process.
 *     The worker follows the strategy pattern.  It is assumed that it has
 *     a mechanism to get work items from something.  The work items consist of
 *     a list of segments.  The first segment is a work item type.
 *     We predefine the following work item types (a concrete class may define
 *     others):
 *
 *     -  PASSTHROUGH_ITEM - these messages have payloads that contain
 *                           items that the workers should just pass through
 *                           to its sink.
 *     -  PROCESS_ITEM     - These messages have payloads that workers should
 *                           process and then send to the sink.
 *     -  END_ITEM         - These items indicate there's no more work items,
 *                           and that the thread should exit.
 *     -  DROP_ITEM        - These items contain payloads that for whatever reason,
 *                           the worker should drop on the floor.
 */

class Worker : public Thread
{
// Message type definitions:

protected:
    static const uint32_t PASSTHROUGH_ITEM = 1;
    static const uint32_t PROCESS_ITEM     = 2;
    static const uint32_t DROP_ITEM        = 3;
    static const uint32_t END_ITEM         = 0xffffffff;
    
    // messages look like this:
    
    typedef struct _Message {
        uint32_t         s_messageType;
        std::list<void*> s_dataParts;
    } Message, *pMessage;
    
    // Canonicals:
public:
    Worker(const char* pName);
    virtual ~Worker();
    
    virtual void run();                // Main flow of control.
    
    
    // The strategy methods:
protected:
    virtual void     connectSource()              = 0;
    virtual void     connectSink()                = 0;
    virtual void     disconnectSource()           = 0;
    virtual void     disconnectSink()             = 0;
    virtual Message  getMessage()                 = 0;
    virtual void FreeMessage(Message& msg)        = 0;
    virtual void* processMessage(Message& msg)    = 0;
    virtual void  freeProcessedMessage(void* pData) = 0;
    virtual void* sendProcessedMessage(Message& msg, void* pData) = 0;
    virtual void* onEnd(Message& msg)             = 0;
    virtual void  onUnknownType(Message& msg) {}
};
#endif