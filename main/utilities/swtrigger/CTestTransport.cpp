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

/** @file:  CTestTransport.cpp
 *  @brief: Implement the test transport class.
 */
#include "CTestTransport.h"
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <stdint.h>


/**
 *  recv
 *     - If there's a message return it to the caller along with its size.
 *     - IF not, throw std::runtime_error.
 *
 *  @param ppData - pointer that will be  filled in with a pointer to malloc(3)'d
 *                  data containing the message.
 *  @param size   Reference to a size_t that will be filled in with the message
 *                  size.
 *  @note If there are no messages, this is treated like most sources treat
 *        an end of data conditions:  size <- 0 and in our case (but not
 *        for all transports, *ppData <- nullptr.)
 *  @note the caller must eventually free(3) the data.
 */
void
CTestTransport::recv(void** ppData, size_t& size)
{
    if (m_messages.empty()) {
        size=0;
        *ppData = nullptr;
    } else {
        auto& msg = m_messages.front();
        size     = msg.size();
        void* pMsg= malloc(msg.size());
        if  (!pMsg) {
            throw std::runtime_error("Memory allocation failed in test transport");
        }
        memcpy(pMsg, msg.data(), size);
        *ppData = pMsg;                     // Set the user pointer.
        m_messages.pop_front();
    }
}
/**
 * send
 *    "Sends" a message. The parts of the message are stored in
 *    a multiPartMessage which is then appended to the m_sendMessages
 *    public member where tests can fish them back out.
 *
 *  @param parts  - iovector describing the message parts.
 *  @param numParts - number of message parts.
 */
void
CTestTransport::send(iovec* parts, size_t numParts)
{
    if (numParts) {
        m_sentMessages.emplace(m_sentMessages.end());
        auto& msg = m_sentMessages.back();
        
        for (int i =0; i < numParts; i++) {
            uint8_t* start = static_cast<uint8_t*>(parts[i].iov_base);
            uint8_t* end   = start + parts[i].iov_len;
            msg.emplace(msg.end(), start, end);
        }
    } else {
        throw std::invalid_argument("CTestTransport::send - there must be at least one message part");
    }
}
/**
 * addMessage
 *    Adds a ne message to the m_message MessageList.
 *
 * @param pData - pointer to the data in the message to add.
 * @param nBytes - Number of bytes of data to add.
 * @throw std::runtime_error if nBytes == 0.
 */
void
CTestTransport::addMessage(void* pData, size_t nBytes)
{
    if (nBytes) {
        uint8_t* pEnd = static_cast<uint8_t*>(pData);
        pEnd += nBytes;
        m_messages.emplace(
            m_messages.end(), static_cast<uint8_t*>(pData), pEnd
        );
        
    } else {
        throw std::runtime_error("Zero length message in test transport");
    }
}