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

/** @file:  CZMQTransport.cpp
 *  @brief: Implement the CZMQTransport class.
 */

#include "CZMQTransport.h"

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <iostream>


// The context singleton:

zmq::context_t* CZMQTransport::m_pContextSingleton(nullptr);

/**
 * destructor
 *    If the socket is not null it is destroyed via delete.
 *    That should close it as well.
 */
CZMQTransport::~CZMQTransport()
{
    if (m_pSocket) delete m_pSocket;
}
/**
 * recv
 *    Recive a (possibily multi-part) message from the peer.
 *    All of the data in the message parts are glommed together into
 *    a single block which is returned to the caller.
 *
 *  @param ppData  Will be filled in with a pointer to the message data
 *                 that was malloc(3)'d in to existence and must be
 *                 free(3)d by the caller when done.
 *  @param size    Reference to a size_t that will be filled in with the
 *                 total number of bytes of data in the received message.
 *  @throw std::invalid_argument - if the socket is null and has not been set.
 */
void
CZMQTransport::recv(void** ppData, size_t& size)
{
    if (m_pSocket) {
        std::vector<zmq::message_t*> messageParts;
        int64_t more(0);
        size_t  moreSize(sizeof(int64_t));
        size_t  totalBytes(0);
        do {
            messageParts.push_back(new zmq::message_t);
            zmq::message_t* msg(messageParts.back());
            m_pSocket->recv(msg);
            totalBytes += msg->size();
            m_pSocket->getsockopt(ZMQ_RCVMORE, &more, &moreSize);
        } while (more);
        // Now marshall the message parts into a buffer:
        
        size    = totalBytes;
        if (totalBytes) {
            uint8_t* pBuffer = static_cast<uint8_t*>(malloc(totalBytes));
            if (!pBuffer) {
                throw std::runtime_error("CZMQTransport::recv - allocation failed");
            }
            *ppData = pBuffer;
            
            
            for (int i = 0; i < messageParts.size(); i++) {
                size_t partSize = messageParts[i]->size();
                memcpy(pBuffer, messageParts[i]->data(), partSize);
                pBuffer += partSize;
                delete messageParts[i];
            }
            uint8_t* pBegin = static_cast<uint8_t*>(*ppData);
            if ((pBuffer - pBegin) > totalBytes) {
                std::cerr << " Bad error: put " << (pBuffer - pBegin)
                    << " bytes into " << totalBytes << std::endl;
            }
        } else {
            *ppData = nullptr;
        }
        
        
    } else {
        throw std::invalid_argument("CZMQTransport::recv - socket is not set.");
    }
}
/**
 * send
 *    Send a multipart message.
 *
 * @param parts - I/O vector describing the parts.
 * @param numPart - number of message parts.
 * @note Zero copy sends are not used.
 * @throw std::runtime_error -if the socket is not set.
 */
void
CZMQTransport::send(iovec* parts, size_t numParts)
{
    if (m_pSocket) {
        for (int i = 0; i < numParts; i++) {
            zmq::message_t part(parts[i].iov_len);
            memcpy(part.data(), parts[i].iov_base, parts[i].iov_len);
            m_pSocket->send(part, i < (numParts-1) ? ZMQ_SNDMORE : 0);
        }
    } else {
        throw std::runtime_error("CZMQTransport::send - socket not set.");
    }
}
/**
 * send end indicator - empty message.
 */
void
CZMQTransport::end()
{
    zmq::message_t m(0);
    m_pSocket->send(m, 0);
}
/**
 * getContext
 *    Return the context singleton:
 *  @retun zmq::context_t*
 */
zmq::context_t*
CZMQTransport::getContext()
{
    if (!m_pContextSingleton) {
        m_pContextSingleton =
            new zmq::context_t(1);          // how to decide thread count?
    }
    return m_pContextSingleton;
}
/**
 * operator zmq::socket_t*
 *    Provide the underlying socket for users with special needs.
 *  @return zmq::socket_t*
 *  @retval nullptr means the socket has not yet been set by the derived transport.
 */
CZMQTransport::operator zmq::socket_t*()
{
    return m_pSocket;
}
/**
 * setSocket
 *   Intended to be used by derived classes to set the base class's socket
 *   once it's been created and connected (or is listening).
 *
 * @param pSocket - pointer to the new socket.
 */
void
CZMQTransport::setSocket(zmq::socket_t* pSocket)
{
    m_pSocket = pSocket;
}