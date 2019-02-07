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

/** @file:  ZMQFanoutBalancedRingSource
 *  @brief: data source that takes ring items from a ZMQFanoutBalancedRingSink
 */
#ifndef ZMQFANOUTBALANCEDRINGSOURCE_H
#define ZMQFANOUTBALANCEDRINGSOURCE_H
#include "CRingItemDataSource.h"
#include <zmq.hpp>

#include <list>

class CRingItemDataSink;
class CRingItem;

/**
 * @class ZMQFanoutBalancedRingSource
 *
 *   This class is intended to get data from a ZMQFanoutBalancedRingSink.
 *   We contain one of those objects in order to be able to interact with
 *   it appropriately.  As a data source we'll produce blocks of data that
 *   are essentially what we get from CRingBlockReader and will marshall
 *   those up into a queue of ring items that can be fetched out one at a time
 *   using getNextItem.
 */
class ZMQFanoutBalancedRingSource : public CRingItemDataSource
{
private:
    CRingItemDataSink*  m_pSink;   // Our data comes from here.
    zmq::socket_t*              m_pSocket; 
    std::list<CRingItem*>       m_itemQueue; // Items we've got from last read.
public:
    ZMQFanoutBalancedRingSource(CRingItemDataSink* sink);
    virtual ~ZMQFanoutBalancedRingSource();

    virtual void connect() ;              //< connect to source.
    virtual void disconnect() ;           //< disconnect from source.
    virtual MessageType::Message
                 getNextItem();           //< Next ring item as message.
    virtual void Register(
        MessageType::Message& regmsg) ;   //< Register as data source.
    virtual void unRegister(
        MessageType::Message& unregmsg
    ) ;                                   //< Unregister as data source.
    
    virtual void* connectSource();
    virtual void  closeSource(void* c);    
private:
    void queueRingItems(MessageType::Message& msg);
    void wrapAndPop(MessageType::Message& msg);
    void formatEndItem(MessageType::Message& msg);
};

#endif