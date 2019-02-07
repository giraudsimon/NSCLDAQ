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

/** @file:  ZMQFanoutBalancedRingSource.cpp
 *  @brief: Impelementation of the ZMQFanoutBalancedRingSource class.
 */

#include "ZMQFanoutBalancedRingSource.h"
#include "CRingItemDataSink.h"
#include <CRingItem.h>
#include <CRingItemFactory.h>
#include <CRingBlockReader.h>
#include <DataFormat.h>

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
/**
 * constructor
 *    Svaes the peer object so that we can use its methods
 * @param sink - our peer that's fanning out items to people just like us.
 */
ZMQFanoutBalancedRingSource::ZMQFanoutBalancedRingSource(
    CRingItemDataSink* sink
) : m_pSink(sink), m_pSocket(nullptr)
{}
/**
 * destructor
 *    If necessary, disconnnects us from our peer.
 */
ZMQFanoutBalancedRingSource::~ZMQFanoutBalancedRingSource()
{
    disconnect();
}

/**
 * connect
 *    Connect to/register with our peer.  This is a no-op if we're already
 *    connected.
 */
void
ZMQFanoutBalancedRingSource::connect()
{
    if (!m_pSocket) {
        void* c = m_pSink->connectSink();
        m_pSocket = static_cast<zmq::socket_t*>(c);
    }
}
/**
 * disconnect
 *    Disconnect/unregister from our peer.  This is a no-op if we're alread
 *    disconnected.
 */
void
ZMQFanoutBalancedRingSource::disconnect()
{
    if (m_pSocket) {
        void* c = m_pSocket;
        m_pSocket = nullptr;
        m_pSink->closeSink(c);
    }               // otherwise no-op
}
/**
 * getNextItem
 *    If we have no items in our item queue, we pull a new set of
 *    items from the peer, and queue them.  We then message-wrap the
 *    ring item at the top of the queue to the user.
 *
 *   Assumptions:
 *   -   We're only getting PROCESS_ITEM and END_ITEM items from the peerl.
 *   -   This implies that if, after the code that tries to stock the queue
 *       we still have an empty queue, we feed an END_ITEM back to the
 *       caller.
 * @returne MessageType::Message the message wrapped ring item or
 *          an END_ITEM message if there's no more data.
 */
MessageType::Message
ZMQFanoutBalancedRingSource::getNextItem()
{
    if (m_itemQueue.empty()) {
        MessageType::Message peerMsg = m_pSink->requestData(m_pSocket);
        if (peerMsg.s_messageType == MessageType::PROCESS_ITEM) {
            queueRingItems(peerMsg);
        }
    }
    // Unless we got an END_ITEM or something pretty illegal,
    // we should have an item to pop.  If not assume we got an end item.
    
    MessageType::Message result;
    if (m_itemQueue.empty()) {
        formatEndItem(result);            // no more data.
    } else {
        wrapAndPop(result);              // Ring item at queue front.
    }
    
    return result;
}
/**
 * Register
 *  - we don't support this so ignore 
 */
void
ZMQFanoutBalancedRingSource::Register(MessageType::Message& msg)
{
}
/**
 * unRegister
 *    We dont' support registration so ignore.
 */
void
ZMQFanoutBalancedRingSource::unRegister(MessageType::Message& msg)
{
}
/**
 *  connectSource
 *     We do the connecting so this throws a logic error if another
 *     thread tries to treat us like a server.
 */
void*
ZMQFanoutBalancedRingSource::connectSource()
{
    throw std::logic_error(
        "ZMQFanoutBalancedRingSource is a client -  connectSource is not supported"
    );
}
/**
 * closeSource
 *     again not supported (see connectSource)
 */
void
ZMQFanoutBalancedRingSource::closeSource(void* c)
{
    throw std::logic_error(
        "ZMQFanoutBalancedRingSource is a client - closeSource not supported"
    );
}
/////////////////////////////////////////////////////////////////////////
// Private utility methods.

/**
 * queueRingItems
 *    Takes a message that has a block of raw ring items, convets them
 *    to CRingItems (using CRingItemFactory) and adds them to the
 *    tail of m_itemQueue where  getNextItem will pull them down for
 *    processing as requested.
 *
 *  @param msg - A PROCESS_ITEM message with two data parts:
 *              -  Number of ring items.
 *              -  A malloced block prefixed by a CRingBlockReader::DataDescriptor
 *                 that describes the data in the remainder of the block.
 */
void
ZMQFanoutBalancedRingSource::queueRingItems(MessageType::Message& msg)
{
    // we don't actually care about the count:
    
    free(msg.s_dataParts.front().second);   // Free the count pointer.
    msg.s_dataParts.pop_front();
    
    CRingBlockReader::pDataDescriptor p =
        static_cast<CRingBlockReader::pDataDescriptor>(
            msg.s_dataParts.front().second
    );
    uint8_t* pCursor = static_cast<uint8_t*>(p->s_pData);
    for (int i =0; i < p->s_nItems; i++) {
        CRingItem* pItem = CRingItemFactory::createRingItem(pCursor);
        m_itemQueue.push_back(pItem);    // processor must delete this.
        
        uint32_t* pCount = reinterpret_cast<uint32_t*>(pCursor);
        pCursor += *pCount;
    }
    free(p);              // Release the storage.
}
/**
 * wrapAndPop
 *    Wraps the front of the item queue in a message and pops it off
 *    the queue.
 *    -  The caller must ensure that the queue is not empty.
 *    -  The message created will be a PROCESS_ITEM
 *    -  The payload will be a single data part.  The
 *       size will be the size of the raw ring item but the
 *       pointer will be a pointer to a CRingItem which must be
 *       deleted when processing is complete
 *
 * @param[out] result  The message that will be filled in.
 */
void
ZMQFanoutBalancedRingSource::wrapAndPop(MessageType::Message& result)
{
    
    CRingItem* pItem = m_itemQueue.front();
    m_itemQueue.pop_front();
    
    result.s_messageType = MessageType::PROCESS_ITEM;
    std::pair<uint32_t, void*> dataPart;
    dataPart.first  = pItem->getItemPointer()->s_header.s_size;
    dataPart.second = pItem;
    result.s_dataParts.push_back(dataPart);
    
}
/**
 *  formatEndItem
 *     Returns an END_ITEM message with  no data parts.
 *  @param[out] result the message that will be formatted.
 */
void
ZMQFanoutBalancedRingSource::formatEndItem(MessageType::Message& result)
{
    result.s_messageType = MessageType::END_ITEM;
    
}