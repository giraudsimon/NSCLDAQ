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

/** @file:  CZMQServerTransport.cpp
 *  @brief: Implements the Zmq server transport specialization.
 */

#include "CZMQServerTransport.h"


/**
 * constructor
 *    - Constructs the socket.
 *    - binds it to the specified URI.
 *    - Sets the parent class's socket.
 */
CZMQServerTransport::CZMQServerTransport(const char* pUri, int socketType)
{
    zmq::socket_t* pSocket =
        new zmq::socket_t(*CZMQTransport::getContext(), socketType);
    pSocket->bind(pUri);
    setSocket(pSocket);
}