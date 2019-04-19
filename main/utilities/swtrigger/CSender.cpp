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

/** @file:  CSender.cpp
 *  @brief: Implement the sender class.
 */
#include "CSender.h"
#include "CTransport.h"


/**
 * constructor
 *     @param transport - references the transport used to send data.
 */
CSender::CSender(CTransport& transport) :
    m_pTransport(&transport)
{}

/**
 *  sendMessage
 *     Send a multipart message:
 *
 *  @param parts -the part speciications.
 *  @param numParts - the number of parts.
 *  @note - not all transports can actually d multipart messages.  Those that
 *          cannot will send a single message containing all parts.
 */
void
CSender::sendMessage(iovec* parts, size_t numParts)
{
    m_pTransport->send(parts, numParts);
}

/**
 * sendMessage
 *    Send a single part message.
 * @param pBase - pointer to the data.
 * @param nBytes- Number of bytes in the messsage
 */
void
CSender::sendMessage(void* pBase, size_t nBytes)
{
    iovec part;
    part.iov_base = pBase;
    part.iov_len  = nBytes;
    
    sendMessage(&part, 1);
}