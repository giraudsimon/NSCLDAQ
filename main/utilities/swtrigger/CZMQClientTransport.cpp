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

/** @file:  CZMQClientTransport
 *  @brief: Implement CZMQClientTransport.
 */
#include "CZMQClientTransport.h"
#include <zmq.hpp>

/**
 * constructor
 *  - create the socket as directed.
 *  -  connect it to the specified URI:
 *
 *  @param pUri - pointer to the URI connection string.
 *  @param socketType  Type of socket to create (e.g. ZMQ_PULL).
 */
CZMQClientTransport::CZMQClientTransport(const char* pUri, int socketType)
{
    zmq::socket_t* pSocket =
        new zmq::socket_t(*CZMQTransport::getContext(), socketType);
    pSocket->connect(pUri);
    setSocket(pSocket);
}