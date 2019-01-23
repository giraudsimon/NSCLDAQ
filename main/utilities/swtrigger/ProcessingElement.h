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

/** @file: ProcessingElement.h
 *  @brief: ABC for a processing element that's in a communicating threaded environment.
 */
#ifndef PROCESSINGELEMENT_H
#define PROCESSINGELEMENT_H

#include <Thread.h>
#include "MessageTypes.h"


/**
 * @class ProcessingElement
 *   This is an abstract element for a processing element in a set of
 *   communicating processes in a thread.  No assumption is made about fanin/fanout
 *   or communication mechanisms... This is a grand strategy pattern class.
 *   The assumption is that processing is a pipeline where each element has a
 *   source of work items and each element has a sink for its work items.
 *   There may be fanout and fan-in -- that's all handled in how the
 *   elements connect to their sources and sinks.
 *
 *   One other policy descision.   Messages exchanged consist of a message type
 *   (this can be expanded from the starting set in MessageTypes.h) and
 *   a list of message segments.  It's up to the application to determine
 *   the ownership of message segments (if they are dynamic when they are deleted).
 */
class ProcessingElement : public Thread
{
protected:
    bool m_running;
public:
    ProcessingElement(const char* name);
    virtual ~ProcessingElement();
    
    virtual void run();                     // Thread entry point.
    
protected:
    //  Terminate and join from the outside world:
    
    void terminateAndJoin();
    
    // These methods are intended to be called from inside the thread and
    // are strategy handlers:
    
           // Communication:
           
    virtual void connectSource()                               = 0;
    virtual void connectSink()                                 = 0;
    virtual void disconnectSource()                            = 0;
    virtual void disconnectSink()                              = 0;
    virtual MessageType::Message getNextWorkItem()             = 0;
    virtual void sendWorkItemToSink(MessageType::Message& workItem) = 0;
        // Processing:
    
    virtual void onRegister(MessageType::Message& reg)         = 0;
    virtual void onUnregister(MessageType::Message& reg)       = 0;
    virtual void processWorkItem(MessageType::Message& item)   = 0;
    virtual void onOtherMessageType(MessageType::Message& item)= 0;
    virtual void onEndItem(MessageType::Message& endItem)      = 0;
    virtual void onExitRequested(MessageType::Message& item)   = 0;
    
    virtual void onDataRequest(MessageType::Message& item);      // Override for explicit pull protocols.
    
    
    
    
    
        // These methods encapsulate how to communicate with this
        // thread:
    
         // Base communication primitive:
public:
    virtual void sendMessageToThread(MessageType::Message& item)      = 0;
    
         // I can see how to implement these in terms of
         // send message by default - but concrete classe may need to override.
         
    virtual void registerClient(std::string identity);
    virtual void unregisterClient(std::string identity);
    virtual void queueWorkItem(std::list<std::pair<uint32_t, void*> >& item);
    virtual void requestItem(std::string identity);   // for explicit pull requests.
    virtual void noMoreData();
    virtual void requestExit();
private:
    void sendMessageWithIdentityToThread(uint32_t type, std::string identity);
};


#endif