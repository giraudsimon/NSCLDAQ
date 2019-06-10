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

/** @file:  CZMQRouterTransport.cpp
 *  @brief: implements the CZMQRouterTransport fanout class.
 */

#include "CZMQRouterTransport.h"
#include "CZMQServerTransport.h"
#include <zmq.hpp>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>

/**
 * construtor:
 *    Create the underlying transport object with the URI desired.
 *
 * @param uri - URI on which we listen for connections.
 */
CZMQRouterTransport::CZMQRouterTransport(const char* pUri) :
    m_pTransport(nullptr)
{
    m_pTransport = new CZMQServerTransport(pUri, ZMQ_ROUTER);
}
/**
 * destructor
 *    Just destroy the transport.  Note that destructors are not
 *    supposed to throw exceptions so if we're destroyed with
 *    active clients -- well try real hard to make sure that does not happen.
 */
CZMQRouterTransport::~CZMQRouterTransport()
{
    delete m_pTransport;
    m_pTransport = nullptr; 
}

/**
 * recv
 *    Not legal for the application, so throw an std::logic_error.
 *
 *  @param ppData - would be where the data pointer goes but...
 *  @param size   - Would be where the amount of data goes but...
 *  @throw std::logic_error - unconditionally.
 */
void
CZMQRouterTransport::recv(void** ppData, size_t& size)
{
    throw std::logic_error(
        "CZMQRouterTransport::recv - cannot recv on a unidirectional sending transport"
    );
}
/**
 * send
 *    - Get the pull request from  a client.
 *    - If the client is not yet knwon, register it.
 *    - Satisfy the request for data.
 *
 *  @param parts - the data message parts.
 *  @param numParts -number of message parts to send.
 *  @throw std::invalid_argument if the pull request is invalid.
 */
void
CZMQRouterTransport::send(iovec* parts, size_t numParts)
{
    uint64_t id = getPullRequest();
    
    // If necessary register the client.
    
    if (!m_clients.hasClient(id)) {
        m_clients.add(id);
    }
    // Send the data to the client:
    
    sendTo(id, parts, numParts);
}

/**
 * end
 *    Send end messages to all clients.  The protocol assumes:
 *    - End messages are those that only consist of the id and delimeter.
 *    - Once a client has received an end message it won't ask for more data.
 *    - All clients eventually ask for data.
 */
void
CZMQRouterTransport::end()
{
    while (!m_clients.empty()) {
        uint64_t id =  getPullRequest();
        iovec v;
        v.iov_base = nullptr;
        v.iov_len  = 0;
        sendTo(id, &v, 1);                // No parts message is end.
        m_clients.remove(id);              // Remove the client.
    }
}
///////////////////////////////////////////////////////////////////////
// Utility methods.

/**
 * sendTo
 *    Send a message to a client given an id.
 *    -  Get the socket from the underlying transport.
 *    -  Send the id as a message part.
 *    -  send the the delimiter.
 *    -  If there are message parts send them via the normal transport.
 *
 *  @param id - id of the destination.
 *  @param parts - pointer to message part descriptions.
 *  @param numParts number of message parts.
 */
void
CZMQRouterTransport::sendTo(uint64_t id, iovec* parts, size_t numParts)
{
    zmq::socket_t* pSock = *m_pTransport; // they have converters for this.
    zmq::message_t idPart(sizeof(uint64_t));
    memcpy(idPart.data(), &id, sizeof(uint64_t));
    zmq::message_t delimPart(0);
 
    pSock->send(idPart, ZMQ_SNDMORE);
    pSock->send(delimPart, numParts ? ZMQ_SNDMORE : 0);
    
    if (numParts) m_pTransport->send(parts, numParts);
    
}
/**
 * getPullRequest
 *   Block until the next pull request:
 *
 *  @return uint64_t - id of the client the request came from.
 */
uint64_t
CZMQRouterTransport::getPullRequest()
{
    uint64_t* pPull;
    size_t   nBytes;
    m_pTransport->recv(reinterpret_cast<void**>(&pPull), nBytes);
    
    // Message must be just a uint64_t (delimieter is zero length).
    
    if (nBytes != sizeof(uint64_t)) {
        throw std::logic_error(
            "CZMQRouterTransport - pull request has invalid format"    
        );
    }
    
    uint64_t result = *pPull;
    free(pPull);
    
    return result;
}