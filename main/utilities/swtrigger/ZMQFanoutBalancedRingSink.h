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

/** @file:  ZMQFanoutBalancedRingSink.h
 *  @brief: Fans out data as pulled by clients. (ZMQ *Router/Dealer)
 */
#ifndef ZMQFANOUTBALANCEDRINGSINK_H
#define ZMQFANOUTBALANCEDRINGSINK_H

#include "CRingItemDataSink.h"
#include <zmq.hpp>
#include <set>
#include <list>
#include <string>
#include <pthread.h>

/**
 * @class ZMQFanoutBalancedRingSink
 *
 *    This class implements a sink that sends ringbuffer data via an
 *    explicitly pulled Router/Dealer ZMQ communication pattern (we're the ROUTER).
 *
 *   To make this work in the framework of a CRingItemDataSink, we need
 *   to
 *   -  Have a queue of messages from which we can satisfy pull requests.
 *   -  Have a queue of pull requests we can send messages to if data comes in.
 *   -  Know when we're done - in which case we send ENDs to all pull requestors.
 *
 *   The scheme requires clients to register with us via REGISTRATION messages,
 *   Pull data via DATA_REQ messages and, when we send them an END_ITEM
 *   unregister via an UNREGISTER message.
 *
 *   Note that registration and unregistration is conveniently hidden by
 *   the connectSink and closeSink calls.  The connection returned,
 *   actually a zmq::socket_t* is then  used by the corresponding data source
 *   to request data using the requestData method we supply.
 *
 *   As is usual, for fanout beasts, we are the server.
 */
class ZMQFanoutBalancedRingSink : public CRingItemDataSink
{
private:
    std::string         m_URI;               // Service name.
    zmq::socket_t*      m_pSocket;           // Socket we route on.
    std::set<pthread_t> m_clients;           // Set of our clients.
    std::list<MessageType::Message*> m_messageQueue;  // queue of un-sent messages.
    std::list<pthread_t> m_dataRequestQueue; // Queue of requests for data.

public:
    ZMQFanoutBalancedRingSink(const char* uri);
    ~ZMQFanoutBalancedRingSink();
    
    virtual void connect();             //< Connect to sink for 1:1 comms.
    virtual void disconnect();          //< Disconnect sink for 1:1 comms.
    virtual void send(MessageType::Message& msg); //< send to peer.
    virtual void onPullRequest(MessageType::Message& msg);
    virtual void Register(
        MessageType::Message& regmsg
    );                                  //< Register as data sink
    virtual void unRegister(
        MessageType::Message& unreg
    );
    virtual bool haveMessage();
    MessageType::Message getMessage();
    
    virtual void* connectSink();         //< If we are server.
    virtual void  closeSink(void* c);  //< If we are server.
    virtual MessageType::Message  requestData(void* c);   //< Request data

private:
    void flushQueue();
    void sendMessage(pthread_t who, MessageType::Message* msg);
    void sendRingItems(pthread_t who, MessageType::Message* msg);
    void freeMessage(MessageType::Message* msg);
    void freeRingMessage(MessageType::Message* msg);
    void queueMessage(MessageType::Message& message);
    pthread_t getClient(MessageType::Message& msg);
    void runDownClients();
    bool more();
    bool more(zmq::socket_t* pSock);  // Could be static actually.
    void sendDataRequest(zmq::socket_t* pSock);
    void sendEnd(pthread_t who);
};

#endif