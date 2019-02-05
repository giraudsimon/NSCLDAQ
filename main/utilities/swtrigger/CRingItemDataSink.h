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

/** @file:  CRingItemDataSink.h
 *  @brief: ABC for class that can send ring items to sinks.
 */
#ifndef CRINGITEMDATASINK_H
#define CRINGITEMDATASINK_H

#include "MessageTypes.h"

/**
 * @interface CRingItemDatasSink
 *    Abstract base class that defines the interfaces needed to
 *    send ring items on to other processors.  Note that it's also just
 *    find for the class to buffer up several work items  for sink processes
 *    before shipping of another.
 */
class CRingItemDataSink
{
public:
    CRingItemDataSink();
    virtual  ~CRingItemDataSink();
    
    virtual void connect() = 0;             //< Connect to sink for 1:1 comms.
    virtual void disconnect() = 0;          //< Disconnect sink for 1:1 comms.
    virtual void send(MessageType::Message& msg) = 0; //< send to peer.
    virtual void onPullRequest(MessageType::Message& msg) = 0;
    virtual void Register(
        MessageType::Message& regmsg) = 0;   //< Register as data sink
    virtual void unRegister(
        MessageType::Message& unreg
    );
    virtual void* connectSink() = 0;         //< If we are server.
    virtual void  closeSink(void* c)   = 0;         //< If we are server.
};

#endif