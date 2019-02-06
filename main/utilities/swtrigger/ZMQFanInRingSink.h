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

/** @file:  ZMQFanInRingSink.h
 *  @brief: ZMQ fan in data sink.
 */
#ifndef ZMQFANINRINGSINK_H
#define ZMQFANINRINGSINK_H
#include "CRingItemDataSink.h"
#include "MessageTypes.h"

#include <zmq.hpp>

class CRingItemDataSource;

/**
 * @class ZMQFanInRingSink
 *    This class is intended to sink data to a ZMQFanInRingSource
 *    That is it provides data to a data source in another thread/process
 *    over ZMQ and is one of many to do it using ZMQ.  It is expected
 *    to send data in the same shape as RingItemBlockConverter so that
 *    blocks of ring items can be sent in a single message.
 *
 *    This sink runs the PUSH side of a PUSH/PULL socket pair, as such,
 *    -  We don't have pull requests.
 *    -  While we get registered with our peer, we don't accept registrations
 *       >from< our peer.
 */

class ZMQFanInRingSink : public CRingItemDataSink
{
private:
    CRingItemDataSource* m_pPeer;
    zmq::socket_t*       m_pSocket;
public:
    ZMQFanInRingSink(CRingItemDataSource* src);
    virtual ~ZMQFanInRingSink();
    
    virtual void connect();             //< Connect to sink for 1:1 comms.
    virtual void disconnect();          //< Disconnect sink for 1:1 comms.
    virtual void send(MessageType::Message& msg); //< send to peer.
    virtual void onPullRequest(MessageType::Message& msg) {}
    virtual void Register(
        MessageType::Message& regmsg) {}   //< Register as data sink
    virtual void unRegister(
        MessageType::Message& unreg
    ) {};
    virtual void* connectSink() {};         //< If we are server.
    virtual void  closeSink(void* c)   {};         //< If we are server.
private:
    void sendRingItems(MessageType::Message& msg);
    void sendNonRingItems(MessageType::Message& msg);
};

#endif