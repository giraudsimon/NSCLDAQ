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
 *  @param nBytes   Reference to a size_t that will be filled in with the message
 *                  size.
 *  @throw std::runtime_error - if there are no messages.
 *  @note the caller must eventually free(3) the data.
 */
void
CTestTransport::recv(void** ppData, size_t& size)
{
    if (m_messages.empty()) {
        throw std::runtime_error("No messages available in test transport");
    } else {
        auto msg = m_messages.front();
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