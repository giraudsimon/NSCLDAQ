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

/** @file:  CZMQRouterTransport.h
 *  @brief: Transport implementing ROUTER as a fanout process.
 */
#ifndef CZMQROUTERTRANSPORT_H
#define CZMQROUTERTRANSPORT_H
#include "CFanoutTransport.h"
#include "CClientRegistry.h"

#include <sys/uio.h>
#include <stdint.h>

class CZMQServerTransport;

/**
 * @class CZMQRouterTransport
 *    Uses the ZMQ router side of a router/dealer pair to implement
 *    a fanout transport.   We send messages to a CZMQDealerTransport.
 *    Data is sent using a pull level protocol.
 *
 *    - CLients send messages that consist of a uint64_t id followed
 *      by a empty delimeter (that we don't see).
 *    - If the cilent is not registered we register it.
 *    - We send data to that client.
 *    - end will collect requests for data and send back empty messages
 *      indicating the end to a client.
 *
 *   @note We derive from the fanout transport but encapsulate a
 *         CZMQServer transport6.
 *   @note recv throws an exception because this transport is considered
 *         unidirectional.
 */
class CZMQRouterTransport : public CFanoutTransport
{
private:
    CClientRegistry      m_clients;
    CZMQServerTransport* m_pTransport;
public:
    CZMQRouterTransport(const char* pUri);
    virtual ~CZMQRouterTransport();
    
    // Transport methods:
    
    virtual void recv(void** ppData, size_t& size);    // throws an exception.
    virtual void send(iovec* parts, size_t numParts);  // send data to a worker.
    virtual void end();                                // no more data.
private:
    void sendTo(uint64_t id, iovec* parts, size_t numParts);
    uint64_t getPullRequest();
};

#endif