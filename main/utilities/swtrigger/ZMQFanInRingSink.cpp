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

/** @file:  ZMQFanInRingSink.cpp
 *  @brief: Implements the methods of the ZMQFanInRingSink class.
 */

#include "ZMQFanInRingSink.h"
#include "CRingItemDataSource.h"
#include <CRingItem.h>
#include <DataFormat.h>
#include <CRingBlockReader.h>


/**
 * constructor
 *   We are cosntructed with a pointer to our peer.
 *  @param src - pointer to the source we communicate with.
 */
ZMQFanInRingSink::ZMQFanInRingSink(CRingItemDataSource* src) :
    m_pPeer(src), m_pSocket(nullptr) {}

/**
 * destructor
 *    Disconnects (if still connected).
 */
ZMQFanInRingSink::~ZMQFanInRingSink()
{
    disconnect();                   // will take care of any disconnecting.
}

/**
 * connect
 *    Connects to our peer by invoking its connectSource method.
 *    That will return a void* that's really a zmq::socket_t*
 */
void
ZMQFanInRingSink::connect()
{
    m_pSocket = static_cast<zmq::socket_t*>(m_pPeer->connectSource());
}
/**
 * disconnect
 *    If we are connected, disconnect by  the peer's
 *    closeSource method.  That also deletes the socket_t.
 */
void
ZMQFanInRingSink::disconnect()
{
    if (m_pSocket) {
        zmq::socket_t* pSock  = m_pSocket;
        m_pSocket = nullptr;
        m_pPeer->closeSource(m_pSocket);
    }
}
/**
 * send
 *   Sends a message to the peer.  The message we get has payload types that
 *   depend somewhat on the message type:
 *   - For all items except PROCESS_ITEM, the message sent is pretty much
 *    the message we get.
 *   - For PROCESS_ITEM, the message parts are assumed to point to
 *     CRingItem objects.  We produce a block of data that has, as a header,
 *     a CRingBlockReader::DataDescriptor followed by storage for the items
 *     in the header.  See sendRingItems and ZMQ::FanInRingSource::fillQueue
 *     for code that shows how this is marshalled and unpacked.
 *
 *   For non PROCESS_ITEM messages, the message parts are free-d
 *   For PROCESS_ITEM messages, the CRingItem objects are deleted.
 */
void
ZMQFanInRingSink::send(MessageType::Message& msg)
{
    if (msg.s_messageType == MessageType::PROCESS_ITEM) {
        sendRingItems(msg);
    } else {
        sendNonRingItems(msg);
    }
}
/**
 * sendRingItems
 *    Sends a message contaning ring items to the peer on m_pSocket.
 *    The incoming message has data parts whose second element is a pointer
 *    to a CRingItem and the first element is the number of bytes in the
 *    ring item.  We're going to produce a message that will be a block of
 *    malloced data.  The front of this data will be a
 *    CRingBlockReader::DataDescriptor.  Its s_pData pointer will point to
 *    the storage that immediately follows that block; which will contain
 *    the unwrapped ring items.
 *    We'll send a message with three parts, a type of MessageType::PROCESS_ITEM
 *    A second part will contain the number of ring items and a
 *    final part will be the block of data we created.
 *
 *    @param msg - the inbound message to send.  The CRingItems pointed to
 *                 by the data parts will be deleted.
 */
void
ZMQFanInRingSink::sendRingItems(MessageType::Message& msg)
{
    // figure out how much data must be allocated:
    
    size_t payloadSize = sizeof(CRingBlockReader::DataDescriptor);
    for (auto p = msg.s_dataParts.begin(); p != msg.s_dataParts.end(); p++) {
        payloadSize += p->first;
    }
    //  To minimize data movement, we'll actually build the data right
    //  in the message:
    
    zmq::message_t payload(payloadSize);
    CRingBlockReader::pDataDescriptor pDesc =
        static_cast<CRingBlockReader::pDataDescriptor>(payload.data());
    pDesc->s_nBytes = payloadSize - sizeof(CRingBlockReader::DataDescriptor);
    pDesc->s_nItems = msg.s_dataParts.size();
    pDesc->s_pData  = &(pDesc[1]);
    
    // Now fill in the data block part:
    
    uint8_t* pCursor = static_cast<uint8_t*>(pDesc->s_pData);
    for (auto p = msg.s_dataParts.begin(); p != msg.s_dataParts.end(); p++) {
        CRingItem* pItem = static_cast<CRingItem*>(p->second);
        memcpy(pCursor, pItem->getItemPointer(), p->first);
        pCursor += p->first;
        delete pItem;
    }
    // Ok now we can send the three message parts that make up the message:
    // We preserve the message type though it should be PROCESS_ITEM
    
    zmq::message_t type(sizeof(uint32_t));
    memcpy(type.data(), &msg.s_messageType, sizeof(uint32_t));
    m_pSocket->send(type, ZMQ_SNDMORE);
    
    // Now the number of items:
    
    zmq::message_t nItems(sizeof(uint32_t));
    memcpy(nItems.data(), &(pDesc->s_nBytes), sizeof(uint32_t));
    m_pSocket->send(nItems, ZMQ_SNDMORE);
    
    m_pSocket->send(payload);
    
}
/**
 * sendNonRingItems
 *     Rebuilds the input message as a  set of zmq::message parts
 *     and sends it on to the peer connected to m_pSocket.
 *
 *  @param msg - the message to send.  The pointers are assumed to point to
 *               malloced storage which is freed.  The message itself is also
 *               emptied of data parts.
 */
void
ZMQFanInRingSink::sendNonRingItems(MessageType::Message& msg)
{
    // First send the message type:
    
    zmq::message_t hdr(sizeof(uint32_t));
    memcpy(hdr.data(), &msg.s_messageType, sizeof(uint32_t));
    m_pSocket->send(hdr, msg.s_dataParts.empty() ? 0 : ZMQ_SNDMORE);
    
    while (!msg.s_dataParts.empty()) {
        std::pair<uint32_t, void*> item = msg.s_dataParts.front();
        msg.s_dataParts.pop_front();
        
        zmq::message_t part(item.first);
        memcpy(part.data(), item.second, item.first);
        free(item.second);
        m_pSocket->send(part, msg.s_dataParts.empty() ? 0 : ZMQ_SNDMORE);
    }
    
}
