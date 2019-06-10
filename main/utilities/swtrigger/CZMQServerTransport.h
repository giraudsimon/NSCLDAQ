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

/** @file:  CZMQServerTransport.h
 *  @brief: Constructs CZMQTransports that require a bind(3zmq).
 */
#ifndef CZMQSERVERTRANSPORT_H
#define CZMQSERVERTRANSPORT_H

#include "CZMQTransport.h"

/**
 * @class CZMQServerTransport
 *    Specialization of a ZMQ transport that acts as a server.
 *    This means that after the socket is created it's bound to a URI
 *    rather than connecting to one as CZMQCLientTransport does.
 */
class CZMQServerTransport : public CZMQTransport
{
public:
    CZMQServerTransport(const char* pUri, int socketType);
    virtual ~CZMQServerTransport() {}
};

#endif