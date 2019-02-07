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

/** @file:  ZMQFanInRingSource.h
 *  @brief: RingItemDataSource that uses ZMQ PUSH/PULL to fan-in.
 */
#ifndef ZMQFANINRINGSOURCE_H
#define ZMQFANINRINGSOURCE_H

#include "CRingItemDataSource.h"
#include <zmq.hpp>
#include <set>
#include <string>
#include <pthread.h>

class CRingItem;

/**
 * @class ZMQFanInRingSource
 *    This class runs the PULL side of a ZMQ PUSH/PULL socket set that's
 *    used to fan in data from multiple sources.  The sources are assumed
 *    to be ring items.   They are assumed to come in batches with messages
 *    of the form produced by the swFilterRingBlockDataSource.
 *
 *    Since this is the singleton end of the PUSH/PULL socket set, this
 *    class is a ZMQ server.  Connecting as a client implies sending a
 *    registration message to us which we use to keep track of the
 *    active data source peers.  Disconnecting as a client sends a similar
 *    unregistration message.  In this implementation, the thread Id is
 *    used as the registration identification.
 *
 *    Since this is push pull, we don't know or, in this case, care where
 *    the data comes from.    End of data happens when there's no buffered data
 *    and there are no more data sources from which to get data.
 *
 *    We assume there will not be a completely empty data source.
 */
class ZMQFanInRingSource : public CRingItemDataSource
{
private:
    zmq::socket_t*      m_pSocket;
    std::string         m_URI;
    std::set<pthread_t> m_clients;
    std::list<CRingItem*>  m_queuedItems;
    bool                m_receivedWorkItems;
public:
    ZMQFanInRingSource(const char* uri);
    virtual ~ZMQFanInRingSource();
    
    virtual void connect();           //< connect to source.
    virtual void disconnect();        //< disconnect from source.
    virtual MessageType::Message
                 getNextItem();       //< Next ring item as message.
    virtual void Register(
        MessageType::Message& regmsg
    );                                //< Register as data source.
    virtual void unRegister(
        MessageType::Message& unregmsg
    );                                //< Unregister as data source.
    
    virtual void* connectSource();    //< Client connect to this server.
    virtual void  closeSource(void* c); //< Client disconnect from this server.
    
    // Utility Methods.
    
private:
    MessageType::Message receiveMessage();
    void fillQueue(MessageType::Message& msg);
    void sendMessage(zmq::socket_t& sock, MessageType::Message& msg);
    std::pair<uint32_t, void*> getTid();
    bool more();
};



#endif