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

/** @file:  CRingItemDataSource.h
 *  @brief: Provide a source of ring items for a CRingItemPipelineProcessor
 *
 */
#ifndef CRINGITEMDATASOURCE_H
#define CRINGITEMDATASOURCE_H
#include "MessageTypes.h"

/**
 * @interface CRingItemDataSource
 *    Abstract base class for source of RingItem work items.
 *    Provides a data source that produces ring items from the communication
 *    peer.  Messages it produces have payloads that consist of a dynamically
 *    allocated ring item.  It is possible an very common for there to be
 *    internal buffering such that a single message will produce
 *    several ring items.
 */
class CRingItemDataSource
{
public:
    CRingItemDataSource();
    virtual ~CRingItemDataSource();
    
public:
    virtual void connect()    = 0;           //< connect to source.
    virtual void disconnect() = 0;           //< disconnect from source.
    virtual MessageType::Message
                 getNextItem() = 0;          //< Next ring item as message.
    virtual void Register(
        MessageType::Message& regmsg) = 0;   //< Register as data source.
    virtual void unRegister(
        MessageType::Message& unregmsg) = 0; //< Unregister as data source.
    
    virtual void* connectSource()       = 0;
    virtual void  closeSource(void* c)  = 0;
    
    
};
#endif