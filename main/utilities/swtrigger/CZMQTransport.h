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

/** @file:  CZMQTransport
 *  @brief: Base class for ZMQ transports.  
 */
#ifndef CZMQTRANSPORT_H
#define CZMQTRANSPORT_H
#include "CTransport.h"

#include <zmq.hpp>
#include "stddef.h"

/**
 * @class CZMQTransport
 *     Base class for the family of ZMQ transports.  These break rougly into
 *     server and client transports which may break further if there are
 *     special needs for the differing socket types (e.g. SUB needs a way to
 *     make subscriptions).
 */
class CZMQTransport : public CTransport {
public:
    zmq::socket_t* m_pSocket;
public:
    CZMQTransport() : m_pSocket(nullptr) {}
    virtual ~CZMQTransport();
    
    void recv(void** ppData, size_t& size);
    
    // ZMQ specific operations.
    
    operator zmq::socket_t*();
protected:
    void setSocket(zmq::socket_t* pSocket);   // derived constructors need this.

};

#endif
