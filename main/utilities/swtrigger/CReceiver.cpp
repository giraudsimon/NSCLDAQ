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

/** @file:  CReceiver.cpp
 *  @brief: Implement receiver class.
 */

#include "CReceiver.h"
#include "CTransport.h"

/**
 * constructor
 *   @param rTransport - references the transport object that's responsible
 *                       for marshalling messages.
 */
CReceiver::CReceiver(CTransport& rTransport) : m_pTransport(&rTransport)
{}


/**
 * getMessage
 *    Gets the next message from the transport layer.
 * @param ppdata pointer to a pointer for the data received. The resulting data
 *        are dynamically allocated via malloc(3) at the transport layer
 *        and must be free(3)'d by the application layer when it's done
 *        with it.
 * @param size - Reference to a size_t that will be filled in by the transport
 *               layer with  the number of bytes of data in the message.
 */
void
CReceiver::getMessage(void** ppData, size_t& size)
{
    m_pTransport->recv(ppData, size);
}
/**
 * setTransport
 *    Allows the client to change the tranpsort associated with a receiver.
 * @param rTransport -new transport.
 * @return CTransport*  - the prior transport.
 * @note any storage management assoiciated with the old transport is up to
 *       the client.
 */
CTransport*
CReceiver::setTransport(CTransport& rTransport)
{
    CTransport* result = m_pTransport;
    m_pTransport = &rTransport;
    return result;
}
