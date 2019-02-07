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

/** @file:  ZMQFanInRingSource.cpp
 *  @brief: Implements the ZMQFanInRingSource class.
 */
#include "ZMQFanInRingSource.h"
#include "ZMQContext.h"

#include <CRingItem.h>
#include <CRingItemFactory.h>
#include <CRingBlockReader.h>

#include <stdexcept>
#include <stdlib.h>
#include <string.h>

/**
 * Constructor
 *     - Initialize the URI so that when connect is called
 *       the service can be established.
 *     - Null out the socket.
 *
 * @param uri - the URI of the service to advertise.
 */
ZMQFanInRingSource::ZMQFanInRingSource(const char* uri) :
    m_pSocket(nullptr), m_URI(uri), m_receivedWorkItems(false)
{}
/**
 * destructor
 *    Turns off the socket.  If the set of queued items or
 *    clients are not empty, an exception (std::logic_error) is thrown
 *    indicating this destruction was premature.
 */
ZMQFanInRingSource::~ZMQFanInRingSource()
{
    
    disconnect();
    if (!m_clients.empty()) {
        throw std::logic_error("Destroying a fan in ring source with connected clients");
    }
    if (!m_queuedItems.empty()) {
        throw std::logic_error("Destorying a fan in ring source with un-processed ring items");
    }
}
/**
 * connect
 *    This establishes the service by
 *    - Creating a PULL zmq::socket_t
 *    - Binding that to m_URI
 *    - Saving the resulting socket in m_pSocket
 *    The fact that m_pSocket is non null when the socket has been
 *    completely set up is used by connectSource
 */
void
ZMQFanInRingSource::connect()
{
    zmq::socket_t* pSocket =
        new zmq::socket_t(ZMQContext::getContext(), ZMQ_PULL);
    pSocket->bind(m_URI.c_str());
    m_pSocket = pSocket;                    // Set only when all set up.
}
/**
 *  disconnect
 *     - Set the m_pSocket as nullptr
 *     - Deletes the socket.
 *
 *     This order does our best to inform connectSource that we're no
 *     longer open for business in  a way that doen't leave a socket pointer
 *     visible to a deleted socket.
 */
void
ZMQFanInRingSource::disconnect()
{
    zmq::socket_t* pSocket;
    m_pSocket = nullptr;                // connectSource sees this.
    
    delete pSocket;                    // does the actual close.
}
/**
 * getNextItem
 *    If there are ring items queued return one to the caller wrapped
 *    in a message.  If not, receive a message.
 *    - If the received message is a PROCESS_ITEM fill the queue with
 *      the ring items in that item and then return the top one.
 *    - Otherwise, return the message to the caller.  We may get called
 *      back in e.g. Register or unRegister.
 *    - If we're called but there are no registrations and the
 *      queue is empty, we return an END_ITEM.
 *      
 * @return MessageType::Message   the message received.
 */
MessageType::Message
ZMQFanInRingSource::getNextItem()
{
    // If we're all done return:
    
    if (m_receivedWorkItems && m_clients.empty()) {
        MessageType::Message msg;
        msg.s_messageType = MessageType::END_ITEM;
        return msg;
    }
    if (m_queuedItems.empty()) {
        MessageType::Message msg = receiveMessage();
        if(msg.s_messageType == MessageType::PROCESS_ITEM) {
            fillQueue(msg);
        } else {
            return msg;
        }
    }
    // The queue presumably has stuff in it now.... else we've got some
    // sort of bug:
    
    if (m_queuedItems.empty()) {
        throw std::logic_error("getNextItem - empty queues when shoudn't be");
    }
    
    // Wrap the front Ring Item in a messgae and return it:
    
    MessageType::Message msg;
    msg.s_messageType = MessageType::PROCESS_ITEM;
    msg.s_dataParts.push_back({sizeof(CRingItem*), m_queuedItems.front()});
    m_queuedItems.pop_front();
    return msg;
}
/**
 * Register
 *    Called when a REGISTRATION message is received. Since only fan in or
 *    fanout is supported, and we are a fan in; the message was sent from
 *    the connectSource method and will contain the pthread_t of the
 *    client connecting.  The client will be registered in the set of
 *    known clients. 
 * @param msg - the message we respond to:
 * @note it's an unreported error for the same thread to register more than once
 *       since an std::set is used to manage registration, such registrations
 *       are essentially a no-op.
 */
void
ZMQFanInRingSource::Register(MessageType::Message& msg)
{
    pthread_t threadId =
        *(reinterpret_cast<pthread_t*>(msg.s_dataParts.front().second));
    free(msg.s_dataParts.front().second);
    m_clients.insert(threadId);      // no-op if already there.
    
}
/**
 * unRegister
 *    Called when an UNREGISTRATION message is received.  The client is removed
 *    from the set of known clients.  If the client does not exist this
 *    is effectively a no-op.
 *
 *  @param msg - the message we're processing.
 */
void
ZMQFanInRingSource::unRegister(MessageType::Message& msg)
{
    pthread_t threadId =
        *(reinterpret_cast<pthread_t*>(msg.s_dataParts.front().second));
    free(msg.s_dataParts.front().second);
    m_clients.erase(threadId);
}
/**
 * connectSource
 *    Called in a thread that wants to send us data.
 *    - Wait for m_pSocket to be non-null
 *    - Connect a new PUSH socket to the server. URI.
 *    - Send a registration message along the socket.
 *    - Return the socket pointer.
 *
 * @return void* - actually a zmq::socket_t*
 */
void*
ZMQFanInRingSource::connectSource()
{
    zmq::socket_t* pSocket =
        new zmq::socket_t(ZMQContext::getContext(), ZMQ_PUSH);
    // Wait for the server socket to be ready
    while (!m_pSocket) {
        sleep(1);
    }
    pSocket->connect(m_URI.c_str());
    MessageType::Message registration;
    registration.s_messageType = MessageType::REGISTRATION;
    registration.s_dataParts.push_back(getTid());
    sendMessage(*pSocket, registration);
    
    return pSocket;
}
/**
 * closeSource
 *    Called in a thread that has been a client of the server.
 *    -  Push an unregistration message to the server.
 *    -  delete the socket.
 *  @param c - actuall a zmq::socket_t*  once called you cannot use this
 *       object as a socket -- or much of anything else for that matter.
 */
void
ZMQFanInRingSource::closeSource(void* c)
{
    zmq::socket_t* pSock = static_cast<zmq::socket_t*>(c);
    MessageType::Message unregistration;
    unregistration.s_messageType = MessageType::UNREGISTRATION;
    unregistration.s_dataParts.push_back(getTid());
    sendMessage(*pSock, unregistration);
    delete pSock;
}
//////////////////////////////////////////////////////////////////////
// Private utilities.

/**
 * receiveMessage
 *    Receives a message on m_pSocket.  Messages are assumed to contain
 *    - A segment that has the message type.
 *    - A sequence of items that will be marshalled into a
 *      size/pointer pairs where the pointers are to malloc'ed memory and
 *      the data copied out of the zmq::message_t's that carried them.
 *
 *  @return MessageType::message - the message received.
 *  @note m_pSocket is used to perform the recvs
 */
MessageType::Message
ZMQFanInRingSource::receiveMessage()
{
    MessageType::Message result;
    zmq::message_t type;
    m_pSocket->recv(&type);
    memcpy(&(result.s_messageType), type.data(), sizeof(result.s_messageType));
    while(more()) {
        zmq::message_t part;
        m_pSocket->recv(&part);
        uint32_t size;
        void*    payload;
        
        size = part.size();
        payload = malloc(size);
        memcpy(payload, part.data(), size);
        
        result.s_dataParts.push_back({size, payload});
    }
    
    return result;
}
/**
 * fillQueue
 *    Given a message who's payloads are pRingItems.  Uses the
 *    ring item factory to create CRingItem's which are then pushed back
 *    into the m_queuedItems.   The payload parts are assumed to come from
 *    swFilterRingBlockDataSource/RingItemBlockConverter.
 *    On return, dynamic storage allocated by the message is deleted.
 *    
 * @param msg - the message containing the ring items to process.
 */
void
ZMQFanInRingSource::fillQueue(MessageType::Message& msg)
{
    // The first part is redundant information:
    
    free(msg.s_dataParts.front().second);   // just number of items.
    CRingBlockReader::pDataDescriptor pDesc =
        static_cast<CRingBlockReader::pDataDescriptor>(
            msg.s_dataParts.back().second
        );
    uint32_t* pItem = static_cast<uint32_t*>(pDesc->s_pData);
    uint8_t*  pCursor = static_cast<uint8_t*>(pDesc->s_pData);
    for (int i =0; i < pDesc->s_nItems; i++)  {
        m_queuedItems.push_back(CRingItemFactory::createRingItem(pCursor));
        
        // Make pointers to the next ring item.
        
        pCursor += *pItem;
        pItem   = reinterpret_cast<uint32_t*>(pCursor);
    }
    
    
    free(pDesc);
}
/**
 * sendMessage
 *    Send a message that consists of a type message part and one message part
 *    for each of the data items.  The contents of the data items are
 *    copied into the zmq::message_t objects and are then freed using
 *    free(3).
 *
 * @param sock - zmq::socket_t& to which the message parts are sent.
 * @param msg  - MessageType::Message& to marshall and send.
 */
void
ZMQFanInRingSource::sendMessage(zmq::socket_t& sock, MessageType::Message& msg)
{
    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &(msg.s_messageType), sizeof(uint32_t));
    sock.send(type, msg.s_dataParts.empty() ? 0 : ZMQ_SNDMORE);
    while (!msg.s_dataParts.empty()) {
        std::pair<uint32_t, void*> item(msg.s_dataParts.front());
        msg.s_dataParts.pop_front();
        
        zmq::message_t part(item.first);
        memcpy(part.data(), item.second, item.first);
        free(item.second);
        
        sock.send(part, msg.s_dataParts.empty() ? 0 : ZMQ_SNDMORE);
    }
}
/**
 * getId
 *    Returns the Thread Id of the currently running thread in a form that
 *    allows it to be incorporated into a message:
 *    
 *  @return std::pair<uint32_t, void*>  first is the sizeof pthread_t,
 *                  second is a pointer to a malloc'ed block containing
 *                  the pthread_t from pthread_self().
 */
std::pair<uint32_t, void*>
ZMQFanInRingSource::getTid()
{
    std::pair<uint32_t, void*> result;
    
    result.first = sizeof(pthread_t);
    result.second = malloc(sizeof(pthread_t));
    pthread_t tid = pthread_self();
    memcpy(result.second, &tid, sizeof(pthread_t));
    
    return result;
}

/**
 *  more
 *     Called during message reception on m_pSocket to see if there
 *     are any  more message parts.
 *
 *  @return bool - true if there are more parts to the message being received.
 */
bool
ZMQFanInRingSource::more()
{
    uint64_t result;
    size_t   resultSize(sizeof(result));
    
    m_pSocket->getsockopt(ZMQ_RCVMORE, &result, &resultSize);
    
    return result != 0;
}