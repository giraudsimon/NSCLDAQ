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

/** @file:  CZMQClientTransport.h
 *  @brief: Concrete derived class from CZMQTransport for clients.
 */
#ifndef CZMQCLIENTTRANSPORT_H
#define CZMQCLIENTTRANSPORT_H
#include "CZMQTransport.h"

/**
 * @class CZMQClientTransport
 *    Specialization of ZMQ transports for transports that must connect
 *    to a server.
 */
class CZMQClientTransport : public CZMQTransport
{
public:
    CZMQClientTransport(const char* pUri, int socketType);
    virtual ~CZMQClientTransport() {}
};

#endif