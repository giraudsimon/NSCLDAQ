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

/** @file:  ZMQFanoutBalancedRingSink.cpp
 *  @brief: Implement the methods of ZMQFanoutBalancedRingSink.
 */

#include "ZMQFanoutBalancedRingSink.h"
#include "ZMQContext.h"
#include <CRingBlockReader.h>
#include "zhelpers.hpp"
#include <CRingItem.h>

#include <stdlib.h>
#include <string.h>
#include <stdexcept>

/**
 * constructor
 *    Just save the URI connect will set up the socket.
 *
 *  @param uri - URI on which we're going to advertise the service.
 */
ZMQFanoutBalancedRingSink::ZMQFanoutBalancedRingSink(const char* uri) :
    m_URI(uri), m_pSocket(nullptr)
{}

/**
 * destructor
 *    If necessary shutdown the socket.
 */
ZMQFanoutBalancedRingSink::~ZMQFanoutBalancedRingSink()
{
    disconnect();
}

/**
 * connect:
 *    If the socket has not yet been created, create it and bind it
 *    to the URI
 */
void
ZMQFanoutBalancedRingSink::connect()
{
    if (!m_pSocket) {
        zmq::socket_t* pSock =
            new zmq::socket_t(ZMQContext::getContext(), ZMQ_ROUTER);
        pSock->bind(m_URI.c_str());
        
        m_pSocket = pSock;
    }
}
/**
 * disconnect
 *   If the socket exists, destroy it.
 */
void
ZMQFanoutBalancedRingSink::disconnect()
{
    // The order used ensures that clients won't connect during
    // our shutdown of the socket.
    
    if (m_pSocket) {
        zmq::socket_t* pSock = m_pSocket;
        m_pSocket = nullptr;
        delete pSock;
    }
}
/**
 * send
 *    Called when we have messages we can deliver.  The message
 *    is added to the message queue.  If there are clients waiting for
 *    messages, the queue is flushed until there are either no more
 *    queued messages or no more pull requests - whichever is first,.
 *
 *  @param msg - The message to send.
 */
void
ZMQFanoutBalancedRingSink::send(MessageType::Message& msg)
{
    queueMessage(msg);
    if (!m_dataRequestQueue.empty()) flushQueue();
}
/**
 * onPullRequest
 *    Called when a client wants data.  The pthread_t is pulled from
 *    the request message and added to the back of the
 *    data request queue.  If there are messages, they are sent until there
 *    are either no more messages queued or there are no clients queued up
 *    waiting for messages...whichever is first.
 */
void
ZMQFanoutBalancedRingSink::onPullRequest(MessageType::Message& msg)
{
    pthread_t peer = getClient(msg);
    m_dataRequestQueue.push_back(peer);
    if(!m_messageQueue.empty()) flushQueue();
}
/**
 * Register
 *    Process registration from one of the peers we're fanning out to.
 *    The thread is put in the client's set.
 * @param msg - the registration message.
 */
void
ZMQFanoutBalancedRingSink::Register(MessageType::Message& msg)
{
    pthread_t client = getClient(msg);
    m_clients.insert(client);
}
/**
 * unRegister
 *   Remove registration from the client list.
 */
void
ZMQFanoutBalancedRingSink::unRegister(MessageType::Message& msg)
{
    pthread_t client = getClient(msg);
    m_clients.erase(client);
}
/**
 * haveMessage
 *   @return bool - true if a recv from m_pSocket will not block.
 */
bool
ZMQFanoutBalancedRingSink::haveMessage()
{
    zmq_pollitem_t item;
    item.socket = (void*)(*m_pSocket);
    item.events = ZMQ_POLLIN;
    zmq_poll(&item, 1, 0);             // Immediate return.
    return (item.revents & ZMQ_POLLIN)  != 0;
}
/**
 * getMessage
 *    Return a message from the input queue.  Messages will consist of:
 *    - id of peer (pthread_t).
 *    - empty message (delimeter)
 *    - Message Type
 *    - Payload parts (if any).
 *
 *  We make the peer id the first message part.
 */
MessageType::Message
ZMQFanoutBalancedRingSink::getMessage()
{
    MessageType::Message result;
    
    zmq::message_t peer;
    zmq::message_t delimeter;
    zmq::message_t type;
    
    m_pSocket->recv(&peer);
    m_pSocket->recv(&delimeter);
    m_pSocket->recv(&type);
    
    // Fill in what we have:
    
    pthread_t* requestor = static_cast<pthread_t*>(malloc(sizeof(pthread_t)));
    memcpy(requestor, peer.data(), sizeof(pthread_t));
    result.s_dataParts.push_back({uint32_t(sizeof(pthread_t)), requestor});
    memcpy(&result.s_messageType, type.data(), sizeof(uint32_t));
    
    // Now the remaining parts:
    
    while (more()) {
        zmq::message_t part;
        m_pSocket->recv(&part);
        uint32_t size = part.size();
        void*    payload = malloc(size);
        memcpy(payload, part.data(), size);
        result.s_dataParts.push_back({size, payload});
    }
    
    
    return result;
}
/**
 * connectSink
 *    Connects a peer to us.
 *    - Waits, if necessary, until m_pSocket is set - at that time;
 *      the server is up.
 *    - Creates a zmq::socket_t connected to the server.
 *    - sets the id of that to our current thread.
 *    - Sends a registration request to the server.
 *  @return void*  actually a zmq::socket_t*
 */
void*
ZMQFanoutBalancedRingSink::connectSink()
{
    zmq::socket_t* result;
    while(!m_pSocket) {
        sleep(1);                // Wait for server setup.
    }
    
    result = new zmq::socket_t(ZMQContext::getContext(), ZMQ_DEALER);
    pthread_t id = pthread_self();
    result->setsockopt(ZMQ_IDENTITY, &id, sizeof(pthread_t));
    result->connect(m_URI.c_str());
    
    // Now that we're connected, we send a registration request.
    // This means just sending the delimeter after the id and the
    // messageType (MessageType::REGISTRATION).
    
    s_sendmore(*result, "");      // Delimeter.
    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &MessageType::REGISTRATION, sizeof(uint32_t));
    result->send(type);
    
    return result;
}
/**
 * closeSink
 *  - Unregister with the server.
 *  - delete the socket
 *
 *  @param c - the connection - actually a zmq::socket_t*
 */
void
ZMQFanoutBalancedRingSink::closeSink(void* c)
{
    zmq::socket_t* pSock = static_cast<zmq::socket_t*>(c);
    
    s_sendmore(*pSock, "");     // Delimeter.
    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &MessageType::UNREGISTRATION, sizeof(uint32_t));
    pSock->send(type);
 
    delete pSock;              // Closes the connection.   
}
/**
 * requestData
 *     Sends a DATA_REQ message to our peer.  We'll  return the message
 *     that satisifed that request.  The reply message will have the following
 *     message parts:
 *     -  empty  - delimeter.
 *     -  message type (uint32_t)
 *     -  optional payload parts.
 *   @param c  - Actually the client zmq::socket_t*
 *   @return MessageType::Message - received marshalled data.
 */

MessageType::Message
ZMQFanoutBalancedRingSink::requestData(void* c)
{
    MessageType::Message result;
    
    zmq::socket_t* pSock = static_cast<zmq::socket_t*>(c);
    sendDataRequest(pSock);
    
    zmq::message_t type;
    pSock->recv(&type);         // Delimeter.
    pSock->recv(&type);         // message type.
    memcpy(&result.s_messageType, type.data(), sizeof(uint32_t));
    
    while(more(pSock)) {
        zmq::message_t part;
        pSock->recv(&part);
        uint32_t bytes = part.size();
        void*    pData = malloc(bytes);
        memcpy(pData, part.data(), bytes);
        result.s_dataParts.push_back({bytes, pData});
    }
    
    return result;
}
///////////////////////////////////////////////////////////////////////////
// Private utilities. We have a lot of these to make main line logic clean.
//

/**
 * flushQueue
 *    Sends data to peers until either there's no more queued data or
 *    there's no client requests queued.
 *    @note - if one of the messages we send is an END_ITEM,
 *            we enter runDownClients()  to send end items to everybody
 *            and handle unregistrations until there are no more clients.
 */
void
ZMQFanoutBalancedRingSink::flushQueue()
{
    while((!m_messageQueue.empty()) && (!m_dataRequestQueue.empty())) {
        // Get the data item -- if it's an end, we need to rundownClients:
        
        MessageType::Message* pMsg = m_messageQueue.front();
        m_messageQueue.pop_back();
        
        if (pMsg->s_messageType = MessageType::END_ITEM) {
            // Should be empty:
            
            if (! m_messageQueue.empty()) {
                throw std::logic_error(
                    "ZMQFanoutBalancedRingSink - END_ITEM wasn't last item to output"
                );
            }
            freeMessage(pMsg);   // We already know m_messageQueue is empty
            runDownClients();    // So the loop will exit.
            
        } else {
            // Figure out who gets this item:
            
            pthread_t dest = m_dataRequestQueue.front();
            m_dataRequestQueue.pop_front();
            
            if (pMsg->s_messageType == MessageType::PROCESS_ITEM) {
                sendRingItems(dest, pMsg);
                freeRingMessage(pMsg);
            } else {
                sendMessage(dest, pMsg);
                freeMessage(pMsg);
            }
        }
    }
}
/**
 * sendMessage
 *    Sends a message to a peer.   The message must not be one with
 *    ring items as memory management for ring items is done with
 *    new and delete while those for other message types are done with
 *    malloc/free.
 *
 *    The message is assumed to have come from the message queue (m_messageQueue).
 *    Once the message is queued to the ZMQ send layer, freeMessage is called
 *    to free storage associated with the message.
 *
 * @param who - the thread id to whom the message should be routed.  Targeting
 *              is done in zmq by prefacing the actual message parts with
 *              the id followed by an empty (delimeter) frame.
 * @param msg - pointer to the message to send.
 */
void
ZMQFanoutBalancedRingSink::sendMessage(
    pthread_t who, MessageType::Message*  msg
)
{
    zmq::message_t id(sizeof(pthread_t));
    memcpy(id.data(), &who, sizeof(pthread_t));
    
    // Header - destination and delimieter.
    
    m_pSocket->send(id, ZMQ_SNDMORE);
    s_sendmore(*m_pSocket, "");
    
    // Send the message type:

    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &(msg->s_messageType), sizeof(uint32_t));
    m_pSocket->send(type, msg->s_dataParts.empty() ? 0 : ZMQ_SNDMORE);
    
    // Send the message data segments.
    
    for (auto p = msg->s_dataParts.begin(); p != msg->s_dataParts.end(); p++) {
        auto pNext = p;
        pNext++;                   // Need to  know if there's a next one.
        
        uint32_t size  = p->first;
        void*    pData = p->second;
        zmq::message_t part(size);
        memcpy(part.data(), pData, size);
        
        m_pSocket->send(
            part, pNext == msg->s_dataParts.end() ? 0 : ZMQ_SNDMORE
        );
    }
    
    freeMessage(msg);
}
/**
 * sendRingItems
 *    routes a message to a fanout peer that has ring items.
 *    Ring items are stored as pointers to CRingItem in the message data
 *    parts.  These are then marshalled into a block that consists of a
 *    header of the form CRingBlockReader::DataDescriptor followed by
 *    the raw unwrapped ring item data.
 *
 *    To send the data, therefore, we must
 *    - Send a hedaer (destination, delimeter)
 *    - Send the message type
 *    - Send the number of ring items.
 *    - Compute the size needed for the header and raw data then create
 *      a final zmq::message_t to hold it.
 *    - Transfer data from the ring items into the payload message.
 *    - Send that messge
 *    - freeRingMessage to free dynamic memor in our input message.
 *
 *  @param who -who we should route the message to.
 *  @param msg - Pointer to the message that is assumed to have been
 *               dequeued from m_messageQueue.
 *               
 */
void
ZMQFanoutBalancedRingSink::sendRingItems(
    pthread_t who, MessageType::Message* msg
)
{
    zmq::message_t id(sizeof(pthread_t));
    memcpy(id.data(), &who, sizeof(pthread_t));
    m_pSocket->send(id, ZMQ_SNDMORE);           // Routing id
    s_sendmore(*m_pSocket, "");                 // Delimeter.
    
    // Now the message type:
    
    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &(msg->s_messageType), sizeof(uint32_t));
    m_pSocket->send(type, ZMQ_SNDMORE);
    
    //Number of ring items.
    
    uint32_t nItems = msg->s_dataParts.size();
    zmq::message_t count(sizeof(uint32_t)); 
    memcpy(count.data(), &nItems, sizeof(uint32_t));
    m_pSocket->send(count, ZMQ_SNDMORE);
    
    // Now figure out how much space we need for the header and for the
    // ring data:
    
    size_t ringItemSize(0);
    for (auto p = msg->s_dataParts.begin(); p != msg->s_dataParts.end(); p++) {
        ringItemSize += p->first;
    }
    
    // Allocate the message; fill in the header and then the ring items, one
    // at a time:
    
    zmq::message_t payload(
        ringItemSize + sizeof(CRingBlockReader::DataDescriptor)
    );
    CRingBlockReader::pDataDescriptor pDesc =
        static_cast<CRingBlockReader::pDataDescriptor>(payload.data());
    pDesc->s_nBytes = ringItemSize;
    pDesc->s_nItems = msg->s_dataParts.size();
    pDesc->s_pData  = pDesc + 1;
    
    uint8_t* pCursor = static_cast<uint8_t*>(pDesc->s_pData);
    for (auto p = msg->s_dataParts.begin(); p != msg->s_dataParts.end(); p++) {
        CRingItem* pItem = static_cast<CRingItem*>(p->second);
        memcpy(pCursor, pItem->getItemPointer(), p->first);
        
        pCursor += p->first;
    }
    
    // Send the last message:
    
    m_pSocket->send(payload);
    
    freeRingMessage(msg);
}

/**
 * freeMessage
 *    Frees the storage associated with a queued messgae that is not
 *    ring items.  The parts of the message have been malloced in to existence
 *    and the message struct itself was new'd.
 * @param msg - pointer to the message to free.
 */
void
ZMQFanoutBalancedRingSink::freeMessage(MessageType::Message* msg)
{
    for (auto p = msg->s_dataParts.begin(); p != msg->s_dataParts.end(); p++) {
        free(p->second);
    }
    delete msg;
}
/**
 * freeRingMessage
 *    Free storage associated with a message whose payload are ring items.
 *    The assumption is that the ring items were created with new as was
 *    the message itself
 *  @param msg - pointer to the message.
 */
void
ZMQFanoutBalancedRingSink::freeRingMessage(MessageType::Message* msg)
{
    for (auto p = msg->s_dataParts.begin(); p != msg->s_dataParts.end(); p++) {
        CRingItem* pItem = static_cast<CRingItem*>(p->second);
        delete pItem;
    }
    free(msg);
}
/**
 * queueMessage
 *    Adds a message to the request queue.  The assumption is that the
 *    message we're passed in may be ephemeral (e.g. automatic in the caller),
 *    but that the data in the message is not, (in fact we require
 *    it be malloced for non ring items and new'd for ring items).
 *    Therefore, we'll new a message into existence, copy and push a pointer
 *    to the copy into the message queue.
 *  @param msg - message to queue.  Note after this call the message is no longer
 *               usable; as we use a non copying destructive list copy.
 */
void
ZMQFanoutBalancedRingSink::queueMessage(MessageType::Message& msg)
{
    MessageType::Message* pMsg = new MessageType::Message;
    pMsg->s_messageType = msg.s_messageType;
    pMsg->s_dataParts.splice(
        pMsg->s_dataParts.end(), msg.s_dataParts
    );
    m_messageQueue.push_back(pMsg);
}
/**
 * getClient
 *   For ZMQ messages that have a client, that have been marshalled into
 *   MessageType::Message objects, the first message type is a pointer
 *   to a pthread_t that is the client's thread identification.  We
 *   return that.
 *
 * @param msg - the marshalled message.
 * @return pthread_t
 */
pthread_t
ZMQFanoutBalancedRingSink::getClient(MessageType::Message& msg)
{
    pthread_t* pResult = static_cast<pthread_t*>(msg.s_dataParts.front().second);
    return *pResult;
}
/**
 * runDownClients:
 *    - Send an END_ITEM message to all data requests in the data queue.
 *    - Enter a loop to get and process messages.
 *      * New data request messages get an END_ITEM back
 *      * New registrations invoke Register
 *      * Unregistrations invoke unRegisteer
 *    - The loop above exits when the client set is empty indicating all
 *      clients have exited.
 *  Thus, when we see an END_ITEM, we hang on to control until all clients
 *  have been given an END_ITEM which causes them unregister and, presumably,
 *  exit.
 */
void
ZMQFanoutBalancedRingSink::runDownClients()
{
    while (!m_dataRequestQueue.empty()) {
        pthread_t to = m_dataRequestQueue.front();
        m_dataRequestQueue.pop_front();
        sendEnd(to);
    }
    
    while (!m_clients.empty()) {
        MessageType::Message m = getMessage();
        if(m.s_messageType == MessageType::REGISTRATION) {
            Register(m);
        } else if (m.s_messageType == MessageType::UNREGISTRATION) {
            unRegister(m);
        } else if (m.s_messageType == MessageType::DATA_REQ) {
            pthread_t to = getClient(m);
            sendEnd(to);
            freeMessage(&m);
        }
    }
    // All clients should be gone so we can return which
    // will probably result in an exit of this thread.
}
/**
 * more
 *    @return bool - if the message currently being read from m_pSocket
 *                   has more message parts.
 */
bool
ZMQFanoutBalancedRingSink::more()
{
    return more(m_pSocket);
}
/**
 * more
 *   @param pSock -socket to check for
 *   @return bool - true if there are more message parts in the message
 *                  currently being read from a socket.
 */
bool
ZMQFanoutBalancedRingSink::more(zmq::socket_t* pSock)
{
    uint64_t haveMore(0);
    size_t   hmSize(sizeof(uint64_t));
    
    pSock->getsockopt(ZMQ_RCVMORE, &haveMore, &hmSize);
    
    return haveMore != 0;
}
/**
 * sendDataRequest
 *    Create a data request for this source on behalf of the client
 *    making the call.
 *
 * @param pSock - socket connecting the client to the server.
 */
void
ZMQFanoutBalancedRingSink::sendDataRequest(zmq::socket_t* pSock)
{
    s_sendmore(*pSock, "");          // Delimeter (out id is auto-sent).
    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &(MessageType::DATA_REQ), sizeof(uint32_t));
    pSock->send(type);
}
/**
 * sendEnd
 *    Sends an END_ITEM message to the peer.  This consists of
 *    - The peer id.
 *    - An empty delimeter message.
 *    - A message containing the uint32_t MessageType::END_DATA
 */
void
ZMQFanoutBalancedRingSink::sendEnd(pthread_t who)
{
    zmq::message_t id(sizeof(pthread_t));
    memcpy(id.data(), &who, sizeof(pthread_t));
    m_pSocket->send(id, ZMQ_SNDMORE);           // id.
    s_sendmore(*m_pSocket, "");                 // delimeter.
    
    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &(MessageType::END_ITEM), sizeof(uint32_t));
    m_pSocket->send(type);
    
}