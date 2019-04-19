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

/** @file:  CTestTransport.h
 *  @brief: Provide a transport that can be used fro testing purposes.
 */
#ifndef CTESTTRANSPORT_H
#define CTESTTRANSPORT_H
#include "CTransport.h"
#include <deque>
#include <vector>
#include <stdint.h>

/**
 * @class CTestTransport
 *      Provides a transport that can be used in testing.  The transport
 *      provides the ability to stock it with messages that can be
 *      received via recv.
 */
class CTestTransport : public CTransport
{
private:
    typedef std::vector<uint8_t> message;
    typedef std::deque<message> messageList;
    
private:
    messageList m_messages;
public:
    
    void recv(void** ppData, size_t& size);
    
    // Methods for test drivers.
    
    void addMessage(void* pData, size_t nBytes);
};

#endif