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

/** @file:  ZMQPushDataSource.h
 *  @brief: Data source that is the push side of a push/pull socket.
 */
#ifndef ZMQPUSHDATASOURCE_H
#define ZMQPUSHDATASOURCE_H
#include "ProcessingElement.h"
#include "MessageTypes.h"
#include <string>
#include <zmq.hpp>


class DataSource;           // Something that gives us data to distribute.
class DataItemConverter;        // Converts data items into message segments.

/**
 * @class ZMQPushDataSource
 *    A ZMQ push data source is one that runs the PUSH side of a zmq
 *    PUSH/PULL socket.  As a data source, its work items don't come
 *    from some communication medium but from an object that's
 *    connected to something like a file or a ring buffer.
 *
 *    As push sockets are one-way we do not implement support for explicit
 *    pulls of data.  Really for the most part we just
 *    get data from the data source, convert it and ship it off.
 *
 *    Anything that tells us what to do is overridden to throw exceptions
 *    again because all we have is a PUSH socket and nothing can be written
 *    into that.
 *
 *    The nasty issues:
 *    *   How to get data from the actual data source.
 *    *   How to express that data as message objects
 *
 *    Are deferred to two objects, a DataItemSource which gets the data and
 *    a DataItemConverter which does said conversion.
 *    
 */
class ZMQPushDataSource : public ProcessingElement
{
private:
    std::string        m_sinkURI;
    zmq::socket_t*     m_pSocket;
    DataSource*    m_pSource;
    DataItemConverter* m_pConverter;
public:
    ZMQPushDataSource(
        const char* threadName,
        std::string uri, DataSource* pSource, DataItemConverter* pConverter
    );
    virtual ~ZMQPushDataSource();
    
protected:
    virtual void connectSource();          // Tells m_pSource to connect.
    virtual void disconnectSource();
    virtual void connectSink();            // Create/bind the socket.
    virtual void disconnectSink();         // Destroys the socket.
    
    virtual MessageType::Message getNextWorkItem(); // uses source/converter.
    virtual void sendWorkItemToSink(MessageType::Message& workItem); // PUSH.
    
    virtual void onRegister(MessageType::Message& reg);   // no-op.
    virtual void onUnregister(MessageType::Message& reg); // no-op.
    virtual void processWorkItem(MessageType::Message& item); // Sends the work item.
    virtual void onOtherMessageType(MessageType::Message& item); // no-op.
    virtual void onEndItem(MessageType::Message& endItem);  // Sends an end item marks exit.
    virtual void onExitRequested(MessageType::Message& item); // faise ->m_running
    
public:
    virtual void*  connectAsSource() ;          // Throw exception
    virtual void   closeSource(void* c);        // throw exception
    virtual void*  connectAsSink();             // Return a zmq::socket.
    virtual void   closeSink(void* c) = 0;
    
    virtual void sendMessageToThread(void * c, MessageType::Message& item); // Throw exception.
protected:
    virtual MessageType::Message receiveMessage();               // No-op.
    
};

#endif