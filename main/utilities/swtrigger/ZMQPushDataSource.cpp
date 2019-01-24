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

/** @file:  ZMQPushDataSource.cpp
 *  @brief: Implement a data source processing element that uses ZMQ_PUSH
 */

#include "ZMQPushDataSource.h"
#include "DataSource.h"
#include "DataItemConverter.h"
#include <zhelpers.hpp>
#include <stdexcept>
#include <ZMQContext.h>
#include <stdlib.h>
#include <string.h>

/**
 * constructor
 *    Constructs the data source.   Just stores the values. The various
 *    connect methods are where the interesting stuff happens.
 *
 *   @param threadName - Name that will be given to the source thread
 *   @param uri        - URI of the service we'll create/bind to.
 *   @param pSource    - Actual source of raw data.
 *   @param pConverter - Knows how to convert data from the source into messages.
 */
ZMQPushDataSource::ZMQPushDataSource(
    const char* threadName, std::string uri, DataSource* pSource,
    DataItemConverter* pConverter
) :
    ProcessingElement(threadName),
    m_sinkURI(uri), m_pSocket(nullptr), m_pSource(pSource),
    m_pConverter(pConverter)
{}
/**
 * destructor
 *   Since we can't ask us to exit, destruction breaks down as follows:
 *   -  If our thread is requesting destruction we just disconnect.
 *   -  If our thread is not requesting destruction - if not running cool.
 *      otherwise, join...and pray it completes.
 */
ZMQPushDataSource::~ZMQPushDataSource()
{
    if (getId() == runningThread()) {
        disconnectSource();
        disconnectSink();
    } else {
        if (m_running) {
            join();            // Hope it actually does exit; no way to request it.
        }
    }
    // Note that disconnectSink deleted the socket
}

/**
 * connectSource
 *    Just ask the data source to connect to whatever it's connected to.
 */
void
ZMQPushDataSource::connectSource()
{
    m_pSource->connect();
}
/**
 * disconnectSource
 *     Just ask the data source to disconnect.
 */
void
ZMQPushDataSource::disconnectSource()
{
    m_pSource->disconnect();
}
/**
 * connectSink
 *     - Create the socket  as ZMQ_PUSH
 *     - bind it to the URI. as a server.
 *
 *   @note it's our convention that Pushers are the servers and pullers the
 *         clients.
 *   @note Throw a logic_error if the socket pointer isn't null.
 */
void
ZMQPushDataSource::connectSink()
{
    if (m_pSocket) {
        throw std::logic_error("Attempt to double connect ZMQPushDataSource sink");
    } else {
        m_pSocket = new zmq::socket_t(ZMQContext::getContext(), ZMQ_PUSH);
        
        //Turn off lingering:
        
        int linger  = 0;
        m_pSocket->setsockopt(ZMQ_LINGER, &linger, sizeof(int));
        
        // Bind it to the endpoint:
        
        m_pSocket->bind(m_sinkURI.c_str());
    }
}
/**
 * disconnectSink
 *    Just delete the socket and and set its pointer to null.
 *    Double disconnection is a no-op.
 */
void
ZMQPushDataSource::disconnectSink()
{
    delete m_pSocket;            
    m_pSocket = nullptr;
}
/**
 * getNextWorkItem
 *    - Retrieve data from the source.
 *    - Convert it to a message.
 *
 *  @return MessageType::Message - the converted message.
 */
MessageType::Message
ZMQPushDataSource::getNextWorkItem()
{
    std::pair<size_t, void*> item = m_pSource->read();
    return (*m_pConverter)(item);
}
/**
 *  sendWorkItemToSink
 *    PUSHes the wor111k item to the socket.  The message contains
 *    a message part which is the message type (uint32_t),
 *    and a message part for each item in the list of size/pointer
 *    values:
 *    -  The pointers are assumed to be dynamically allocated by the
 *       converter.  They are copied into zmq::message_t rather than dealing
 *       with knowing when to free them (a later refinement/optimization can
 *       run this in zero copy mode if this is a bottleneck).
 *    -  Nothing to stop the converters from producing a pointer to a pointer
 *       of dynamically or statically allocated storage to reduce data
 *       movement as long as we stay inside a single process address space.
 *
 * @param workItem - the message to send.
 */
void
ZMQPushDataSource::sendWorkItemToSink(MessageType::Message& workItem)
{
    
    
    // The message type.  Note that C++11 says std::list::size() is constant
    //complexity.
    
    zmq::message_t typeSegment(sizeof(uint32_t));
    memcpy(typeSegment.data(), &workItem.s_messageType, sizeof(uint32_t));
    m_pSocket->send(
        typeSegment, workItem.s_dataParts.size() ? ZMQ_SNDMORE : 0 );
    
    // Now send any message segments:
    
    while(!workItem.s_dataParts.empty()) {
        std::pair<uint32_t, void*>& item = workItem.s_dataParts.front();
        
        zmq::message_t msg(item.first);
        memcpy(msg.data(), item.second, item.first);
        workItem.s_dataParts.pop_front();
        m_pSocket->send(msg, workItem.s_dataParts.empty() ? 0 : ZMQ_SNDMORE);
        
        free(item.second);
    }
}
/**
 * onRegister
 *    no-op as we don't support registration.  This should never get called
 *    because sendMessageToThread will throw and presumably the converter
 *    won't produce messages of this type.
 *    
 * @param msg - registration message
 */
void ZMQPushDataSource::onRegister(MessageType::Message& msg) {}

/**
 * onUnregister
 *    This too is a no-op  - see onRegister
 *    
 * @param msg unregistration mesg.
 */
void ZMQPushDataSource::onUnregister(MessageType::Message& msg) {}

/**
 * processWorkItem
 *   This is trivial, we just send the work item on to the sink.
 *
 *  @param workItem - the message we got.
 */
void
ZMQPushDataSource::processWorkItem(MessageType::Message& workItem)
{
    sendWorkItemToSink(workItem);
}
/**
 * onOtherMessage
 *    We assume the translator doesn't produce funky types so we're a no-op
 *    (should we throw instead).
 *
 * @param msg -the message.
 *
 */
void
ZMQPushDataSource::onOtherMessageType(MessageType::Message& msg)
{
    
}
/**
 * onEndItem
 *   We end items just get forwarded on.  We're assuming that push
 *   is used a  pure pipeline.  For fanout parallelism via push/pull
 *   we need another class that also provides a secondary communication
 *   path for registration as we'd want to push the onEnd item so that
 *   everyone gets it.
 *
 *  A better match, however is Router/Dealer which we take up in a different
 *  class.
 *
 * @param msg - the end message.
 */
void
ZMQPushDataSource::onEndItem(MessageType::Message& endItem)
{
    sendWorkItemToSink(endItem);
}
/**
 * onExitRequested
 *   This is not really possible from the outside world but it _is_
 *   theoretically possible from the data source (e.g. suppose the
 *   application has a 'one-shot' mode).  We just set the running flag false.
 *   That will make the main loop exit.
 *
 * @param msg - The exit request message - ignored.
 */
void
ZMQPushDataSource::onExitRequested(MessageType::Message& item)
{
    m_running = false;    // defined in base class as protected.
}

//////////////////////////////////////////////////////////////////
// Public interface for requests into the processing element.
// note that we're not capable of having any so we just
//throw exceptions.
//
/**
* connectAsSource
*     Connect to application as data source - well we don't actually
*     have a communicating thread/process as a data source.
*/
void*
ZMQPushDataSource::connectAsSource()
{
    throw std::logic_error(
        "Attempted source connection to ZMQPushDataSource not possible"
    );
}
/**
 * closeSource
 *   If you can't connect you can't close.
 *
 * @param c - supposed connection.
 */
void
ZMQPushDataSource::closeSource(void* c)
{
    throw std::logic_error(
        "Attempted source disconnect from ZMQPushDataSource not possible"
    );
}
/**
 * connectAsSink
 *    We don't support connection as a sink as we don't have a channel
 *    on which ti do this.
 */
void*
ZMQPushDataSource::connectAsSink()
{
    throw std::logic_error(
        "Attempted sink connection to ZMQPushDataSource not possible"
    );
}
/**
 * closeSink
 *    Also not supported.
 */
void
ZMQPushDataSource::closeSink(void* c)
{
    throw std::logic_error(
        "Attempted sink disconnect from ZMQPushDataSource not possible"
    );
}
/**
 * sendMessageToThread
 *    We can't send message to the thread.
 *
 *  @param c -  supposed connection.
 *  @param item - message the client is mistakenly trying to send.
 */
void
ZMQPushDataSource::sendMessageToThread(
    void* c, MessageType::Message& item
)
{
    throw std::logic_error(
        "Attempted to send a message to a ZMQPushDataSource thread no possible"
    );
}
/**
 * receiveMessage
 *    Input isn't message based, the data source does it for us.
 */
MessageType::Message
ZMQPushDataSource::receiveMessage()
{
    throw std::logic_error(
        "Call to receiveMessage by a ZMQPushDataSource thread not allowed"
    );
}